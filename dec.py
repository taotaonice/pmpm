# -*- coding: utf-8 -*-
# tcp mapping created by hutaow(hutaow.com) at 2014-08-31
 
import socket
import threading
import time
import uuid
import random
 
# 端口映射配置信息
CFG_REMOTE_IP = 'eth-us.sparkpool.com'
CFG_REMOTE_PORT = 3333
CFG_LOCAL_IP = '0.0.0.0'
CFG_LOCAL_PORT = 1083
 
# 接收数据缓存大小
PKT_BUFF_SIZE = 2048
 
# 调试日志封装
def send_log(content):
  print(content)
  return
 
def enctry(s):
  s = s.decode()
  k = 'djq%5cu#-jeq15abg$z9_i#_w=$o88m!*alpbedlbat8cr74sd' * 40
  encry_str = ""
  for i,j in zip(s,k):
    # i为字符，j为秘钥字符
    temp = str(ord(i)+ord(j))+'_' # 加密字符 = 字符的Unicode码 + 秘钥的Unicode码
    encry_str = encry_str + temp
  return encry_str.encode()

def dectry(p):
  p = p.decode()
  k = 'djq%5cu#-jeq15abg$z9_i#_w=$o88m!*alpbedlbat8cr74sd' * 40
  dec_str = ""
  for i,j in zip(p.split("_")[:-1],k):
    # i 为加密字符，j为秘钥字符
    temp = chr(int(i) - ord(j)) # 解密字符 = (加密Unicode码字符 - 秘钥字符的Unicode码)的单字节字符
    dec_str = dec_str+temp
  return dec_str.encode()

# 单向流数据传递
def tcp_mapping_worker(conn_receiver, conn_sender, dec):
  cnt = 0
  while True:
    try:
      data = conn_receiver.recv(PKT_BUFF_SIZE)
      if dec:
          if data[0] != 1:
              continue
          data = dectry(data[1:])
    except Exception:
      send_log('Event: Connection closed.')
      break
 
    if not data:
      send_log('Info: No more data is received.')
      break
 
    try:
      if not dec:
        cnt += 1
        data = chr(1).encode() + enctry(data)
        conn_sender.sendall(data)
        if cnt > 5:
            fake_message_num = random.randint(500, 1000)
            for i in range(fake_message_num):
              fake_data = ''
              for j in range(5, 10):
                fake_data += str(uuid.uuid4())
              fake_data = chr(0).encode() + fake_data.encode()
              conn_sender.sendall(fake_data)
      else:
        conn_sender.sendall(data)
    except Exception:
      send_log('Error: Failed sending data.')
      break
 
    # send_log('Info: Mapping data > %s ' % repr(data))
    send_log('Info: Mapping > %s -> %s > %d bytes.' % (conn_receiver.getpeername(), conn_sender.getpeername(), len(data)))
 
  conn_receiver.close()
  conn_sender.close()
 
  return
 
# 端口映射请求处理
def tcp_mapping_request(local_conn, remote_ip, remote_port):
  remote_conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
 
  try:
    remote_conn.connect((remote_ip, remote_port))
  except Exception:
    local_conn.close()
    send_log('Error: Unable to connect to the remote server.')
    return
 
  threading.Thread(target=tcp_mapping_worker, args=(local_conn, remote_conn, True)).start()
  threading.Thread(target=tcp_mapping_worker, args=(remote_conn, local_conn, False)).start()
 
  return
 
# 端口映射函数
def tcp_mapping(remote_ip, remote_port, local_ip, local_port):
  local_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  local_server.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
  local_server.bind((local_ip, local_port))
  local_server.listen(5)
 
  send_log('Event: Starting mapping service on ' + local_ip + ':' + str(local_port) + ' ...')
 
  while True:
    try:
      (local_conn, local_addr) = local_server.accept()
    except (KeyboardInterrupt, Exception):
      local_server.close()
      send_log('Event: Stop mapping service.')
      break
 
    threading.Thread(target=tcp_mapping_request, args=(local_conn, remote_ip, remote_port)).start()
 
    send_log('Event: Receive mapping request from %s:%d.' % local_addr)
 
  return
 
# 主函数
if __name__ == '__main__':
  tcp_mapping(CFG_REMOTE_IP, CFG_REMOTE_PORT, CFG_LOCAL_IP, CFG_LOCAL_PORT)
