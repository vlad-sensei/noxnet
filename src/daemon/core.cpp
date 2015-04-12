#include "core.h"
#include <ratio>
Core_ptr core;

// -------- Constructors ----
Core::Core(){}

void Core::run(uint16_t ui_port){
 thread network_thread([this](){
   Connection_initiator_base::start_listen();
   Connection_initiator_base::run();
 });
 thread ui_thread([this,ui_port](){
   ui = make_unique<UI>();
   ui->run(ui_port);
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


void Core::handle_new_connection(tcp::socket socket){
  debug("new peer..");
  //TODO:check if we are already connected by MAC/IP parameters
  //maybe cache those in the future
  spawn_peer(socket);
}

void Core::req_chunks(const Id &bid, const unordered_set<Id> &cids){
   debug("req_chunks");
  r_lock l(peers_mtx);
  for(const auto& it:data.peers){
    const Peer_ptr& peer = it.second;
    peer->req_chunks(bid, cids);
  }
}

//TODO fix starting after stopping
void Core::start_synch(int period){
    //Capturing this might lead to a dangling pointer if core is destroyed
    should_sync = true;

    if (!sync_thread_exists){
      sync_thread = thread([this, period](void){
          while(should_sync){
            this_thread::sleep_for(chrono::seconds(period));
            core->synch_all();
          }
      });
      sync_thread_exists = true;
    }
}

void Core::stop_synch(){
    should_sync = false;
}

//-------- syncing ----
void Core::synch_all(){
  r_lock l(peers_mtx);
  for(const auto& it:data.peers){
    const Peer_ptr& peer = it.second;
    peer->req_metaheads();
  }
}

// ----------- Data -----------
void Core::spawn_peer(tcp::socket &socket){
  w_lock l(peers_mtx);
  const peer_id_t&  pid = ++data.current_peer_id;
  data.peers[pid] = Peer_ptr(new Peer(socket, pid));
  data.peers[pid]->init();
}

bool Core::remove_peer(const peer_id_t &pid){
  w_lock l(peers_mtx);
  if(!data.peer_exists(pid)) return false;
  data.peers.erase(pid);
  return true;
}
