#include "core.h"
#include <ratio>
Core_ptr core;

// -------- Constructors ----
Core::Core() : ui(unique_ptr<Ui>(new Ui)){}

void Core::run(){
  thread Inventory_thread([this](){
    init_inventory();
  });
  thread Database_thread([this](){
    init_db();
  });

  Inventory_thread.join();
  Database_thread.join();

  thread network_thread([this](){
    Connection_initiator_base::start_listen();
    Connection_initiator_base::run();
  });
  thread ui_thread([this](){
    ui->run();
  });

  network_thread.join();
  ui_thread.join();
}

void Core::connect(const string &addr, const uint16_t &port){
  debug("connecting [%s:%d]..",addr.c_str(),port);
  Connection_initiator_base::connect(addr,port);
}

void Core::broadcast_echo(const string &msg){
  r_lock l(peers_mtx);
  for(const auto& it:data.peers){
    const Peer_ptr& peer = it.second;
    peer->echo(msg);
  }
}


void Core::handle_new_connection(socket_ptr socket){
  debug("new peer [%s]..", socket->lowest_layer().remote_endpoint());
  //TODO:check if we are already connected by MAC/IP parameters
  if(!socket->lowest_layer().is_open()) {debug(" *** socket is not open"); return;}
  if(!socket->lowest_layer().remote_endpoint().address().is_v4()){
    debug(" *** only accepting ipv4 clients at this moment");
    return;
  }

  //maybe cache those in the future
  if(!spawn_peer(socket)) {debug(" *** could not spawn new peer");};
}

void Core::req_chunks(const Id &bid, const unordered_set<Id> &cids){
  debug("req_chunks");
  r_lock l(peers_mtx);
  for(const auto& it:data.peers){
    const Peer_ptr& peer = it.second;
    peer->req_chunks(bid, cids);
  }
}

//-------- syncing ----
//TODO fix starting after stopping
void Core::start_synch(int period){
  //Capturing this might lead to a dangling pointer if core is destroyed

  w_lock l(sync_mtx);
  data.should_sync = true;

  if (data.sync_thread_exists) return;
  data.sync_thread = thread([this, period](void){
    r_lock l(sync_mtx, defer_lock_type());
    while(true){
      this_thread::sleep_for(chrono::seconds(period));
      l.lock();
      if (data.should_sync){
        l.unlock();
        core->synch_all();
      } else l.unlock();
    }
  });
  data.sync_thread_exists = true;
}

void Core::stop_synch(){
  w_lock l(sync_mtx);
  data.should_sync = false;
}

void Core::synch_all(){
  r_lock l(peers_mtx);
  for(const auto& it:data.peers){
    const Peer_ptr& peer = it.second;
    peer->req_metaheads();
  }
}


// ----------- Routing ---------

bool Core::merge_peers(const peer_id_t &pid1, const peer_id_t &pid2){
  //verify they exist
  r_lock l(peers_mtx);
  if(!data.peer_exists(pid1)||!data.peer_exists(pid2)) {
    //debug("*** peers did not exist [%s] [%s]",pid1 ,pid2);
    return false;
  }
  const Peer_ptr& peer1 = data.peers[pid1];
  const Peer_ptr& peer2 = data.peers[pid2];
  uint16_t port1, port2;
  if(!peer1->get_listen_port(port1) && !peer2->get_listen_port(port2)) {
    //debug("*** no listen port from the peer",pid1 ,pid2);
    return false;
  }
  peer1->merge_peer(peer2->get_ip(),port2);
  peer2->merge_peer(peer1->get_ip(),port1);
  return true;
}

void Core::share_peers(uint16_t max_count,const peer_id_t& pid){
  int counter=0;
  r_lock l(peers_mtx);
  vector<peer_id_t> peer_list;
  for(const auto& iterator:data.peers){
    if (pid == iterator.first) continue;
    peer_list.emplace_back(iterator.first);
    if (++counter == max_count) break;
  }
  l.unlock();
  for(auto i:peer_list){
    core->merge_peers(i,pid);
  }
}

bool Core::make_peer_req(const peer_id_t &pid){
  r_lock l(peers_mtx);
  if(!data.peer_exists(pid)) {
    //debug("*** peers did not exist [%s] [%s]",pid1 ,pid2);
    return false;
  }
  const Peer_ptr& peer = data.peers[pid];
  peer->req_peers();
  return true;
}

// ----------- Data -----------
//TODO: consider diabling/tweaking for testing
bool Core::spawn_peer(socket_ptr &socket){
  w_lock l(peers_mtx);
  const ip_t& ip = socket->lowest_layer().remote_endpoint().address().to_v4().to_ulong();
#ifndef ALLOW_PEER_FROM_SAME_IP
  //if(data.peer_ip_exists(ip)) return false;
#endif
  const peer_id_t&  pid = ++data.current_peer_id;
  data.peer_ips[ip]=pid;
  data.peers[pid] = Peer_ptr(new Peer(socket, pid));
  data.peers[pid]->init();
  return true;
}

bool Core::remove_peer(const peer_id_t &pid){
  w_lock l(peers_mtx);
  if(!data.peer_exists(pid)) return false;
#ifndef ALLOW_PEER_FROM_SAME_IP
  //data.peer_ips.erase(data.peers[pid]->get_ip());
#endif
  data.peers.erase(pid);
  return true;
}
