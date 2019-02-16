#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unet/detail/raw_socket.hpp>
#include <unet/detail/socket_set.hpp>
#include <unet/event.hpp>

namespace unet {
namespace detail {

using testing::InSequence;
using testing::MockFunction;
using testing::StrictMock;
using testing::Test;

using MockCallback = StrictMock<MockFunction<void(std::uint32_t)>>;

static const std::string kMessage = "Knock knock, who's there?";

class RawSocketTest : public Test {
 public:
  void SetUp() override {
    socket =
        new RawSocket{RawSocket::kEthernet,
                      1500,
                      1500,
                      1500,
                      std::make_shared<Serializer>(EthernetAddr{}, Ipv4Addr{}),
                      sockets,
                      ss,
                      cb.AsStdFunction()};
    socket->subscribedEventMaskAdd(Event::Send | Event::Read);
  }

  void SendMessage() {
    auto sendLen =
        socket->send(reinterpret_cast<const std::uint8_t*>(kMessage.data()),
                     kMessage.size());
    ASSERT_EQ(sendLen, kMessage.size());
  }

  void ReadMessage() {
    std::uint8_t buf[1024];
    auto readLen = socket->read(buf, sizeof(buf));
    ASSERT_EQ(readLen, kMessage.size());
    ASSERT_EQ((std::string{reinterpret_cast<char*>(buf), readLen}), kMessage);
  }

  SocketSet ss;
  List<RawSocket> sockets;
  MockCallback cb;
  RawSocket* socket = nullptr;
};

TEST_F(RawSocketTest, SendCallback) {
  {
    InSequence s;
    EXPECT_CALL(cb, Call(eventAsInt(Event::Send))).Times(2);
  }

  ss.dispatch();

  // Exhaust the send queue...
  std::size_t messages = 1500 / kMessage.size();
  for (std::size_t m = 0; m < messages; m++) {
    ASSERT_NO_FATAL_FAILURE(SendMessage());
  }

  ss.dispatch();

  for (std::size_t m = 0; m < messages; m++) {
    auto f = socket->popFrame();
    ASSERT_TRUE(f);
    ASSERT_EQ(*f, kMessage);
  }

  ASSERT_FALSE(socket->popFrame());

  ss.dispatch();
}

TEST_F(RawSocketTest, ReadCallback) {
  {
    InSequence s;
    EXPECT_CALL(cb, Call(eventAsInt(Event::Send))).Times(1);
    EXPECT_CALL(cb, Call(Event::Send | Event::Read)).Times(2);
    EXPECT_CALL(cb, Call(eventAsInt(Event::Send))).Times(1);
  }

  ss.dispatch();

  socket->process(*Frame::makeStr(kMessage));
  socket->process(*Frame::makeStr(kMessage));

  ss.dispatch();

  ASSERT_NO_FATAL_FAILURE(ReadMessage());

  ss.dispatch();

  ASSERT_NO_FATAL_FAILURE(ReadMessage());

  ss.dispatch();
}

TEST_F(RawSocketTest, Close) {
  {
    InSequence s;
    EXPECT_CALL(cb, Call(eventAsInt(Event::Send))).Times(1);
  }

  ss.dispatch();

  SendMessage();
  socket->close();

  ss.dispatch();

  // Socket is destroyed inline once the last egress frame is popped.
  socket->popFrame();

  ss.dispatch();
}

}  // namespace detail
}  // namespace unet
