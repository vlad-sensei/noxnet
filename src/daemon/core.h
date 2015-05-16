#ifndef CORE_H
#define CORE_H

/*
 * Core
 *
 * Core is the core class of the program.
 * It is resposible for high-level program
 * and control of its subclasses. It also
 * listens for incoming connections in order
 * to spawn more Peer objects.
 */

#include <memory>
#include <thread>
#include <random>
#include <unordered_map>
#include <mutex>


#include <boost/asio.hpp>

#include "glob.h"
#include "connection_initiator_base.h"
#include "message.h"
#include "peer.h"
#include "ui.h"
#include "library.h"


#define DEBUG

class Core;
typedef unique_ptr<Core> Core_ptr;

class Ui;
typedef unique_ptr<Ui> Ui_ptr;

extern Core_ptr core;

//not synchronized!
struct File_req{
  File_req(const Id& bid_,time_t time):bid(bid_),time_stamp(time){
      chunks[bid_]={};
  }
  File_req(time_t time):time_stamp(time){}
  File_req(){}
  Id bid;
  time_t time_stamp;
  unordered_map<Id,deque<peer_id_t> > chunks;
  unsigned writer_count = 0;
  bool has_metabody = false;
  inline bool chunk_exists(const Id& cid) {return chunks.find(cid)!=chunks.end();}
  inline void insert(const Id& cid) {chunks[cid]={};}
  inline bool erase(const Id& cid){
    if(!chunk_exists(cid)) return false;
    chunks.erase(cid);
    return true;
  }
  inline bool add_peer(const Id& cid,peer_id_t peer_id,time_t time){
      if(!chunk_exists(cid)) return false;
      chunks[cid].emplace_back(peer_id);
      time_stamp=time;
      return true;
  }
  bool get_peer_id(const Id& cid,peer_id_t& peer_id){
      if(!chunk_exists(cid) || chunks[cid].empty()) {
          debug("*** no peer have left a ack");
          return false;
      }
      peer_id=chunks[cid].front();
      chunks[cid].pop_front();
      chunks[cid].emplace_back(peer_id);
      return true;
  }
};
//not synchronized!

struct Inidirect_File_req{
  Inidirect_File_req(const Id& bid_,time_t time):bid(bid_),time_stamp(time){
      chunks[bid_]={};
  }
  Inidirect_File_req(time_t time):time_stamp(time){}
  Inidirect_File_req(){}
  Id bid;
  time_t time_stamp;
  unordered_map<Id,unordered_set<peer_id_t>> chunks;
  unsigned writer_count = 0;
  bool has_metabody = false;
  inline bool chunk_exists(const Id& cid) {return chunks.find(cid)!=chunks.end();}
  inline void insert(const Id& cid) {chunks[cid]={};}
  inline bool erase(const Id& cid){
    if(!chunk_exists(cid)) return false;
    chunks.erase(cid);
    return true;
  }
  void remove_peer(peer_id_t peer_id){
      for(auto& it:chunks){
          it.second.erase(peer_id);
      }
  }

};


class Core : Connection_initiator_base, public Library {
public:
  Core();
  inline void set_daemon_port(const uint16_t& port){Connection_initiator_base::set_port(port);}
  inline uint16_t get_daemon_port() const {return Connection_initiator_base::get_port();}
  inline void set_client_port(const uint16_t& port){ui->set_port(port);}
  void run();
  bool remove_peer(const peer_id_t& pid);
  void ai_run();

  // user interaction (UI)
  void connect(const string& addr, const uint16_t& port);
  void broadcast_echo(const string& msg);
  //start syncing at regular intervalls
  void start_synch(int period);
  //stop syncing after the next syncing is completed
  void stop_synch();
  bool merge_peers(const peer_id_t& pid1, const peer_id_t& pid2);
  void share_peers(uint16_t max_count, const peer_id_t &pid);
  void set_database_path(const string path){Database::set_database_path(path);}
  bool make_peer_req(const peer_id_t &pid1);

  bool req_file(const Id &mid, Id &bid);
  bool req_file_from_peers(const Id &bid);

  void handle_aggresiv_query(const Id& bid,const unordered_set<Id>& cids,peer_id_t pid);
  void handle_chunk(const Id& bid, const Chunk& chunk);
  void handle_chunk_ack(const Id& bid,const unordered_set<Id>& cids,peer_id_t pid);

private:
  void req_chunks(const Id& bid, const unordered_set<Id>& cids);
  void synch_all();
  bool spawn_peer(socket_ptr& socket);
  void handle_new_connection(socket_ptr socket);



  //all data&methods in data must be synchronized
  struct {
    //peers
    peer_id_t current_peer_id = 0;
    unordered_map<peer_id_t,Peer_ptr> peers;
    unordered_map<ip_t, peer_id_t> peer_ips;
    inline bool peer_exists(const peer_id_t& pid) {return peers.find(pid)!=peers.end();}
    inline bool peer_ip_exists(const ip_t& ip_v4){return peer_ips.find(ip_v4)!=peer_ips.end();}

    //synch
    bool should_sync = SYNC;
    bool sync_thread_exists = false;
    thread sync_thread;

    //chunk requests
    map<time_t,Id>file_reqs_time;
    unordered_map<Id, File_req > file_reqs;
    unordered_map<Id, Inidirect_File_req > indirect_reqs;
    inline bool file_req_exists(const Id& bid){ return file_reqs.find(bid) != file_reqs.end();}
    inline bool chunk_req_exists(const Id& bid,const Id& cid){ return file_req_exists(bid) && file_reqs[bid].chunk_exists(cid);}

    inline bool indirect_file_req_exists(const Id& bid){ return indirect_reqs.find(bid) != indirect_reqs.end();}
    inline bool indirect_chunk_req_exists(const Id& bid,const Id& cid){ return indirect_file_req_exists(bid) && indirect_reqs[bid].chunk_exists(cid);}

  } data;
  rw_mutex peers_mtx, pid_mtx, sync_mtx, chunk_req_mtx;

  Ui_ptr ui;
};

#endif // CORE_H
