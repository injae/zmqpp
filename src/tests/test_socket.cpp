/*
 *  Created on: 8 Aug 2011
 *      Author: @benjamg
 */

#include <boost/test/unit_test.hpp>

#include "zmqpp/context.hpp"
#include "zmqpp/socket.hpp"
#include "zmqpp/message.hpp"

BOOST_AUTO_TEST_SUITE( socket )

const int max_poll_timeout = 100;

void wait_for_socket(zmqpp::socket& socket)
{
	zmq_pollitem_t item = { socket, 0, ZMQ_POLLIN, 0 };
	int result = zmq_poll(&item, 1, max_poll_timeout);
	BOOST_REQUIRE_MESSAGE(result >= 0, "polling command returned without expected value: " << zmq_strerror(zmq_errno()));
	BOOST_REQUIRE_MESSAGE(0 != result, "polling command returned with timeout after " << max_poll_timeout << " milliseconds");
	BOOST_REQUIRE_MESSAGE(1 == result, "polling command claims " << result << " sockets have events but we only gave it one");
	BOOST_REQUIRE_MESSAGE(item.revents & ZMQ_POLLIN, "events do not match expected POLLIN event: " << item.revents);
}

BOOST_AUTO_TEST_CASE( socket_creation )
{
	zmqpp::context context;
	zmqpp::socket socket(context, zmqpp::socket_type::pull);
}

BOOST_AUTO_TEST_CASE( socket_creation_bad_type )
{
	zmqpp::context context;
	BOOST_CHECK_THROW(zmqpp::socket socket(context, static_cast<zmqpp::socket_type>(-1)), zmqpp::zmq_internal_exception)
}

BOOST_AUTO_TEST_CASE( simple_pull_push )
{
	zmqpp::context context;

	zmqpp::socket puller(context, zmqpp::socket_type::pull);
	puller.bind("inproc://test");

	zmqpp::socket pusher(context, zmqpp::socket_type::push);
	pusher.connect("inproc://test");

	BOOST_CHECK(pusher.send("hello world!"));

	wait_for_socket(puller);

	std::string message;
	BOOST_CHECK(puller.receive(message));

	BOOST_CHECK_EQUAL("hello world!", message);
	BOOST_CHECK(!puller.has_more_parts());
}

BOOST_AUTO_TEST_CASE( multipart_pair )
{
	zmqpp::context context;

	zmqpp::socket alpha(context, zmqpp::socket_type::pair);
	alpha.bind("inproc://test");

	zmqpp::socket omega(context, zmqpp::socket_type::pair);
	omega.connect("inproc://test");

	BOOST_CHECK(alpha.send("hello", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(alpha.send("world", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(alpha.send("!"));

	wait_for_socket(omega);

	std::string message;

	BOOST_CHECK(omega.receive(message));
	BOOST_CHECK_EQUAL("hello", message);
	BOOST_REQUIRE(omega.has_more_parts());

	BOOST_CHECK(omega.receive(message));
	BOOST_CHECK_EQUAL("world", message);
	BOOST_REQUIRE(omega.has_more_parts());

	BOOST_CHECK(omega.receive(message));
	BOOST_CHECK_EQUAL("!", message);
	BOOST_CHECK(!omega.has_more_parts());
}

BOOST_AUTO_TEST_CASE( subscribe_helpers )
{
	zmqpp::context context;

	zmqpp::socket publisher(context, zmqpp::socket_type::publish);
	publisher.bind("inproc://test");

	zmqpp::socket subscriber(context, zmqpp::socket_type::subscribe);
	subscriber.connect("inproc://test");
	subscriber.subscribe("watch1");
	subscriber.subscribe("watch2");

	BOOST_CHECK(publisher.send("watch0", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(publisher.send("contents0"));
	BOOST_CHECK(publisher.send("watch1", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(publisher.send("contents1"));
	BOOST_CHECK(publisher.send("watch2", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(publisher.send("contents2"));
	BOOST_CHECK(publisher.send("watch3", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(publisher.send("contents3"));

	wait_for_socket(subscriber);

	std::string message;
	BOOST_CHECK(subscriber.receive(message));
	BOOST_CHECK_EQUAL("watch1", message);
	BOOST_REQUIRE(subscriber.has_more_parts());
	BOOST_CHECK(subscriber.receive(message));
	BOOST_CHECK_EQUAL("contents1", message);
	BOOST_CHECK(!subscriber.has_more_parts());

	wait_for_socket(subscriber);

	BOOST_CHECK(subscriber.receive(message));
	BOOST_CHECK_EQUAL("watch2", message);
	BOOST_REQUIRE(subscriber.has_more_parts());
	BOOST_CHECK(subscriber.receive(message));
	BOOST_CHECK_EQUAL("contents2", message);
	BOOST_CHECK(!subscriber.has_more_parts());

	subscriber.unsubscribe("watch1");

	BOOST_CHECK(publisher.send("watch0", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(publisher.send("contents0"));
	BOOST_CHECK(publisher.send("watch1", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(publisher.send("contents1"));
	BOOST_CHECK(publisher.send("watch2", zmqpp::socket::SEND_MORE));
	BOOST_CHECK(publisher.send("contents2"));

	wait_for_socket(subscriber);

	BOOST_CHECK(subscriber.receive(message));
	BOOST_CHECK_EQUAL("watch2", message);
	BOOST_REQUIRE(subscriber.has_more_parts());
	BOOST_CHECK(subscriber.receive(message));
	BOOST_CHECK_EQUAL("contents2", message);
	BOOST_CHECK(!subscriber.has_more_parts());
}

BOOST_AUTO_TEST_CASE( sending_messages )
{
	zmqpp::context context;

	zmqpp::socket pusher(context, zmqpp::socket_type::push);
	pusher.bind("inproc://test");

	zmqpp::socket puller(context, zmqpp::socket_type::pull);
	puller.connect("inproc://test");

	zmqpp::message message;
	std::string part("another world");

	message.add("hello world!");
	message.add(part);

	pusher.send(message);
	BOOST_CHECK_EQUAL(0, message.parts());

	wait_for_socket(puller);

	BOOST_CHECK(puller.receive(part));
	BOOST_CHECK_EQUAL("hello world!", part);
	BOOST_REQUIRE(puller.has_more_parts());
	BOOST_CHECK(puller.receive(part));
	BOOST_CHECK_EQUAL("another world", part);
	BOOST_CHECK(!puller.has_more_parts());
}

BOOST_AUTO_TEST_CASE( receiving_messages )
{
	zmqpp::context context;

	zmqpp::socket pusher(context, zmqpp::socket_type::push);
	pusher.bind("inproc://test");

	zmqpp::socket puller(context, zmqpp::socket_type::pull);
	puller.connect("inproc://test");

	zmqpp::message message;
	std::string part("another world");

	message.add("hello world!");
	message.add(part);

	pusher.send(message);
	BOOST_CHECK_EQUAL(0, message.parts());

	wait_for_socket(puller);

	BOOST_CHECK(puller.receive(message));
	BOOST_CHECK_EQUAL(2, message.parts());
	BOOST_CHECK_EQUAL("hello world!", message.get(0));
	BOOST_CHECK_EQUAL("another world", message.get(1));
	BOOST_CHECK(!puller.has_more_parts());
}

BOOST_AUTO_TEST_CASE( cleanup_safe_with_pending_data )
{
	zmqpp::context context;

	zmqpp::socket pusher(context, zmqpp::socket_type::push);
	pusher.bind("inproc://test");

	zmqpp::socket puller(context, zmqpp::socket_type::pull);
	puller.connect("inproc://test");

	zmqpp::message message;
	std::string part("another world");

	message.add("hello world!");
	message.add(part);

	pusher.send(message);
	BOOST_CHECK_EQUAL(0, message.parts());
}

BOOST_AUTO_TEST_SUITE_END()