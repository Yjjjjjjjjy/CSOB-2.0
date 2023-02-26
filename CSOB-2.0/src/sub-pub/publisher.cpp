#include "redis_publisher.h"

int main(int argc, char *argv[])
{
  CRedisPublisher publisher;

  bool ret = publisher.init();
  if (!ret)
  {
    printf("Init failed.\n");
    return 0;
  }

  ret = publisher.connect();
  if (!ret)
  {
    printf("connect failed.");
    return 0;
  }

  // while (true)
  // {
  //   publisher.publish("test-channel", "Test message");
  //   sleep(1);
  // }
  publisher.publish("test-channel", "Test message");
  for (int i = 0; i < 10; i++){
    publisher.publish("test-channel", std::to_string(i*1000));
    
  }
  sleep(1);

  publisher.disconnect();
  publisher.uninit();
  return 0;
}