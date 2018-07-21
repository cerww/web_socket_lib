#include <boost/asio.hpp>
#include "networking_coro_stuff.h"
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <string>
#include <string_view>
#include "client_hub.h"
#include "server_hub.h"
#include <iostream>
#include <boost/beast/websocket.hpp>
#include <variant>
#include "task.h"

template<typename it>
bool includes(it a,it end_a,it b,it end_b) {
	while(a!=end_a && b!=end_b) {
		if (std::distance(b, end_b) > std::distance(end_a, a))
			return false;

		if (*b < *a) {			
			return false;
		}else if (*a < *b) {
			++a;//increment a until *a == *b or *b < *a, where *b isn't in [a,end_a)
		}else {//*a == *b
			++a;
			++b;
		}
	}
	return b==end_b;
}


template<typename K,typename V>
struct robin_hash_table{
	robin_hash_table() {
		m_entries.resize(16);
	}
	struct entry {		
		entry(entry&& other)noexcept {
			std::copy((char*)&other, ((char*)&other) + sizeof(other), (char*)this);			
			other.distance_from_optimal = -1;
		}

		entry(const entry& other) {
			if(other.distance_from_optimal!=-1) {
				data = other.data;
				distance_from_optimal = other.distance_from_optimal;
			}
		}
		entry& operator=(entry&& other)noexcept {
			~entry();
			std::copy((char*)&other, ((char*)&other) + sizeof(other), (char*)this);
			other.distance_from_optimal = -1;
			return *this;
		}

		entry& operator=(const entry& other) {
			~entry();
			if (other.distance_from_optimal != -1) {
				data = other.data;
				distance_from_optimal = other.distance_from_optimal;
			}
			return *this;
		}

		~entry() {
			if (distance_from_optimal != -1)
				data.~pair<K, V>;
		}
		int distance_from_optimal = -1;
		union {
			std::pair<K, V> data;
			int nothing{};
		};
	};

	struct iterator {
		iterator(typename std::vector<entry>::iterator t):m_it(t){}

	private:
		typename std::vector<entry>::iterator m_it;
	};


private:
	typename std::vector<entry>::iterator hash_value(const K& key) {
		size_t hashed_val = std::hash<K>()(key);
		int current_distance_from_optimal = 0;
		for (int i = hashed_val % m_entries;; ++i) {
			if (m_entries[i].distance_from_optimal == current_distance_from_optimal && m_entries[i].data.first == key) {
				return m_entries.begin() + i;
			}
			else if (m_entries[i].distance_from_optimal == -1) {
				return m_entries.begin() + i;
			}
			else if (m_entries[i].distance_from_optimal<current_distance_from_optimal) {
				return m_entries.begin() + i;
			}
			++current_distance_from_optimal;
		}
	}

	std::pair<K, V>&  insert__(K key) {
		size_t hashed_val = std::hash<K>()(key);
		std::pair<K, V> item = std::make_pair(std::move(key), V{});
		std::pair<K, V>* ret_val = nullptr;
		int current_idx = 0;
		for (int i = hashed_val % m_entries.size();; ++i) {
			if (m_entries[i].distance_from_optimal == current_idx && m_entries[i].data.first == key) {
				return m_entries[i].data;
			}else if(m_entries[i].distance_from_optimal == -1) {
				m_entries[i].distance_from_optimal = i;
				m_entries[i].data = std::move(item);
				if (!ret_val) 
					ret_val = &m_entries[i].data;
				return *ret_val;
			}else if (m_entries[i].distance_from_optimal > current_idx) {
				std::swap(m_entries[i].distance_from_optimal, current_idx);
				std::swap(m_entries[i].data, item);
				if(!ret_val) 
					ret_val = &m_entries[i].data;				
			}
		}
	}

	double m_max_load_factor = 0.5;
	std::vector<entry> m_entries;
};

void a() {
	std::variant<int, std::string> rawrland;
	std::future<int> rawr;
	int i = std::visit([](auto&&...) {return 1; }, rawrland);
}

promise<int> potato;

task<int> charmander() { co_return 2; }

task<void> rawr() {
	auto t = potato.get_task();
	std::cout << co_await t << std::endl;
	co_await charmander();
}

task<void> do_stuff(boost::asio::ip::tcp::socket s,std::atomic<bool>& continue_) {
	boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(std::move(s));
	co_await ws.async_accept(use_task);
	//std::string rawrland = "raw2131231223233123r";
	//co_await ws.async_write(boost::asio::buffer(rawrland),use_task);
	boost::beast::multi_buffer buffery;
	while(continue_) {
		std::string abc = "123456732123123";
		size_t n = co_await ws.async_read(buffery,use_task);
		size_t n2 = co_await ws.async_write(boost::asio::buffer(abc), use_task);
		buffery.consume(n);
	}

}

int main() {

	client_hub<basic_traits> rawr;
	//uWS::Hub rawr;

	boost::asio::io_context ioc;
	boost::asio::ssl::context ctx{ boost::asio::ssl::context::tls };

	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), 3000);

	//boost::asio::ip::tcp::acceptor acceptor(ioc,ep);
	//boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_sock(ioc, ctx);
	server_hub<basic_traits> servery;

	int i = 0;
	int u = 0;
	
	rawr.on_msg = [&](auto& w,std::string_view s){
		w.async_send(std::string(s), [&](auto&&...) {++i; });
	};

	std::atomic<bool> continue_ = true;

	servery.on_msg = [&](auto& w,std::string_view s){
		w.async_send(std::string(s), [&](auto&&...) {++u; });
	};

	rawr.on_connect = [&](auto& w){		
		w.async_send(std::string("12345645erdtfgyhvt876gyuhj789011"), [](auto&&...) {});
	};


	if (auto ec = servery.bind(ep); ec) { std::cout << ec << std::endl; std::cin.get(); };

	servery.listen();

	/*
	acceptor.async_accept([&](boost::system::error_code ec,boost::asio::ip::tcp::socket sock){
		do_stuff(std::move(sock),continue_);
	});
	acceptor.async_accept([&](boost::system::error_code ec,boost::asio::ip::tcp::socket sock){
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(std::move(s));
		ws.accept();
		std::string rawrland = "raw2131231223233123r";		
		boost::beast::multi_buffer buffery;
		while(continue_) {
			size_t n = ws.read(buffery);
			size_t n2 = ws.write(buffery);
			buffery.consume(n);
		}

	});
	*/

	rawr.async_connect("3000://localhost/");
	//rawr.connect("3000://localhost/");

	std::thread t_a([&](){
		servery.run();
	});

	std::thread b([&]() {rawr.run(); });
	

	//uWS::Hub h;
	std::this_thread::sleep_for(std::chrono::seconds(10));
	continue_ = false;
	std::cout << i << std::endl;
		
	std::cin.get();
	t_a.join();
	b.join();
}
