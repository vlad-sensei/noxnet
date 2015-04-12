#include "connection.h"

void Connection::init(){
  Connection_base::run();
}

void Connection::process_msg(const Msg_ptr &msg){
  switch(msg->get_type()){
  case Message::T_ECHO:
    handle_echo(msg);
    break;
  default:
    debug("unknown message type : %d", msg->get_type());
    break;
  }
//  cv.notify_one(); //notify_all()?
}

void Connection::handle_echo(const Msg_ptr &msg){
  const string& echo_str = msg->get_string(Message::K_BODY);
  ui->echo(echo_str);
}
