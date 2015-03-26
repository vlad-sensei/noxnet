#include "database.h"

Database::Database(): Sqlite3_base(DEFAULT_DATABASE_PATH){
  debug("creating database..");
  init_db();
}

Database::~Database(){
  debug("destroying database..");
}

void Database::init_db(){
  debug("initializing database..");
  remove_db(); // for clear for testing

  //create tables
  //creat table for mids
  exec_s(C_METAHEADS);
}

void Database::get_all_metaheads(vector<Metahead>& metaheads){
  debug("getting all..");
  Result_ptr res = exec_q(Q_ALL_METAHEADS);
  while(res->next()) {
    Metahead tmp(res->get_id(2),res->get_string(1));
    metaheads.emplace_back(tmp);
  }
}

bool Database::get_metahead(const Id& mid, Metahead& metahead){
  debug("getting metahead..");
  Result_ptr res = exec_q(Q_METAHEAD,mid);
  if(!res->next())return false;
  metahead.mid = mid;
  metahead.tags = res->get_string(0);
  metahead.bid = res->get_id(1);
  return true;
}


void Database::get_mids_by_tag_pattern(const string& pattern, vector<Id>& mids){
  Result_ptr res = exec_q(Q_MIDS_BY_TAG_PATTERN,pattern);
  while(res->next()) mids.emplace_back(res->get_id(0));
}