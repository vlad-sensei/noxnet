#ifndef LIBRARY_H
#define LIBRARY_H

/*
 * Libary
 *
 * Library represents the programs file data. It is
 * responsible for keeping file data and metadata
 * up to date, operating at SQLite backend.
 * It provides API for seach, upload and download
 * queries.
 */

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "glob.h"
#include "common.h"
#include "inventory.h"


class Library: public Inventory {
public:

  Library();
  //pattern ex Batman%movie%...
  //TODO: right now you need to search the tags in the same order in which it was written.
  void search(const string& pattern, vector<Id>& mids);
  bool upload_file(const string& filename, const string& tags = "");

  bool req_file(const Id& mid);

  //TODO:
  vector<Metahead> publish_metaheads(){
    r_lock l(metaheads_mtx);
    return data.published_metaheads;
  }

  void handle_chunk(const Id& bid, const Chunk& chunk);

  void run_test_uploader();
  void run_test_downloader();
  void add_metahead(const Metahead & metahead){
    //TODO: metahead in ram ?
    Database::add_metahead(metahead);
  }

private:
  virtual void req_chunks(const Id& bid, const unordered_set<Id>& cids) = 0; //request chunks
  bool get_metahead(const Id& mid, Metahead& Metahead);

  struct {
    unordered_map<Id, unordered_set<Id> > chunk_reqs; // chunk_req_map[mid] == set of chunks we are waiting for for that file
    unordered_set<Id> has_metabody_; //rename?

    inline bool file_req_exists(const Id& bid){ return chunk_reqs.find(bid) != chunk_reqs.end();}
    inline bool file_req_exists_and_not_empty(const Id& bid){
      return chunk_reqs.find(bid) != chunk_reqs.end() && !chunk_reqs[bid].empty();
    }
    inline bool chunk_req_exists(const Id& bid,const Id& cid){ return (file_req_exists(bid) && chunk_reqs[bid].find(cid) != chunk_reqs[bid].end() );}
    inline bool has_metabody(const Id& bid){return has_metabody_.find(bid) !=has_metabody_.end();}

    Metahead m = Metahead(Id("a"), "Batman");
    Metahead m1 = Metahead(Id("b"), "Spiderman");
    Metahead m2 = Metahead(Id("c"), "Pop eye");
    vector<Metahead> published_metaheads = { m, m1, m2 };

  } data;


  rw_mutex chunk_reqs_mtx, metaheads_mtx;

};

#endif // LIBRARY_H
