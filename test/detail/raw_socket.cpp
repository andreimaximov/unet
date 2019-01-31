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

static const boost::string_view kMessage = "Knock knock, who's there?";

class RawSocketTest : public Test {
 public:
  void SetUp() override {
    socket =
        new RawSocket{RawSocket::kEthernet, 2,       2,  1500,
                      EthernetAddr{},       sockets, ss, cb.AsStdFunction()};
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
    ASSERT_EQ(boost::string_view(reinterpret_cast<char*>(buf), readLen),
              kMessage);
  }

  SocketSet ss;
  List<RawSocket> sockets;
  MockCallback cb;
  RawSocket* socket = nullptr;
};

TEST_F(RawSocketTest, SendCallback) {
  {
    InSequence s;
    EXPECT_CALL(cb, Call(eventAsInt(Event::Send))).Times(3);
  }

  ss.dispatch();

  ASSERT_NO_FATAL_FAILURE(SendMessage());
  ASSERT_NO_FATAL_FAILURE(SendMessage());

  ss.dispatch();

  auto f1 = socket->popFrame();
  ASSERT_TRUE(f1);
  ASSERT_EQ(*f1, kMessage);

  ss.dispatch();

  auto f2 = socket->popFrame();
  ASSERT_TRUE(f2);
  ASSERT_EQ(*f2, kMessage);

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
