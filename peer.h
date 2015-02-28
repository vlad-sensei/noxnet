#ifndef PEER_H
#define PEER_H

/*
 * Peer
 *
 * Peer represents the peer we are connected to and
 * provides relevant API such as send function and
 * various message handlers. Obviously it also contains
 * data about our peer, such as ID.
 *
 * Every Peer object has an open tcp socket connected
 * to the actual peer, and lives only as long as that
 * socket is open.
 */


#include <memory>
#include <thread>
#include <mutex>
#include <deque>

#include <boost/asio.hpp>

#include "glob.h"
#include "message.h"

namespace bs = boost::system;
typedef boost::asio::ip::tcp::socket socket_t;

class Peer;
typedef shared_ptr<Peer> Peer_ptr;

class Peer {
public:
  Peer(socket_t& sock_, const peer_id_t& pid);
  ~Peer();

  void send(Msg_ptr msg);
private:
  Peer();

  //TODO:  make Peer_network class
  void do_read_header();
  void do_read_body();
  void do_write();
  void do_close();
  void handle_connection_error(const string& location, const bs::error_code& ec);

  Msg_ptr read_msg, write_msg;
  deque<Msg_ptr> msg_queue;
  mutex write_msg_mtx;
  bool msg_is_writing = false;
  socket_t sock;


  //process messages
  void process_msg(Msg_ptr msg);
  void handle_echo(const Msg_ptr& msg);

  struct Data{
    Data(const peer_id_t& pid_):pid(pid_){}
    const peer_id_t pid;
  private:
    Data();
  } data;
};

#endif // PEER_H
