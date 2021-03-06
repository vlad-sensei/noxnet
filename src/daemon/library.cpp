#include "library.h"

#include <boost/filesystem.hpp>

Library::Library() {
  debug("initializing Library..");
}

bool Library::upload_file(const string& file_path, const string& tags,Id& mid){
  Metahead metahead;
  if(!Inventory::upload_file(file_path,metahead,tags))return false;
  add_metahead(metahead);
  mid=metahead.mid;
  return true;
}

void Library::search(const string& pattern, vector<Id>& mids){
  get_mids_by_tag_pattern(pattern,mids);
}

bool Library::get_metahead(const Id& mid, Metahead& metahead){
  return Database::get_metahead(mid,metahead);
}

vector<Metahead> Library::publish_metaheads(){
  vector<Metahead> res;
  Database::get_all_metaheads( res);
  return res;
}

void Library::handle_chunk(const Id& bid, const Chunk& chunk){
  w_lock l(chunk_reqs_mtx);
  if (!data.chunk_req_exists(bid,chunk.cid)){
    debug("there is no reques for this:\n[bid %s]\n[chunk.cid %s]\n",bid,chunk.cid);
    return;
  }
  data.chunk_reqs[bid].erase(chunk.cid);
  l.unlock();
  if(!add_chunk(bid,chunk)) return;
  l.lock();
  if(data.file_req_exists_and_not_empty(bid)){
    //debug("*** have more to get from the network");
    return;
  }
  //build metabody OR we are done
  if(data.has_metabody(bid)){
    data.chunk_reqs.erase(bid);
    l.unlock();
    get_file(bid,"TODO_get_name_of_file");
    return;
  }
  l.unlock();

  Metabody metabody;
  if(!get_metabody(bid,metabody)){
    debug("*** need more chunks for a metabody");
    l.lock();
    data.chunk_reqs[bid].insert(metabody.bid_next());
    req_chunks(bid,data.chunk_reqs[bid]);
    return;
  }else{
    data.has_metabody_.insert(bid);
  }
  l.lock();

  for(Id& cid:metabody.cids){
    data.chunk_reqs[bid].emplace(cid);
  }
  req_chunks(bid,data.chunk_reqs[bid]);
}

bool Library::req_file(const Id& mid,Id& bid){
  Metahead metahead;
  if(!get_metahead(mid,metahead)){
    debug("*** no mid found");
    return false;
  }
  //debug("req_file with:\n[mid %s]\n[bid %s]",mid,metahead.bid);
  data.chunk_reqs[metahead.bid]={metahead.bid};
  debug("*** TODO: check if file is on local pc");
  req_chunks(metahead.bid,data.chunk_reqs[metahead.bid]);
  bid=metahead.bid;
  return true;
}
