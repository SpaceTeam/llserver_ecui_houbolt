#ifndef SOCKET_HPP
#define SOCKET_HPP

class Socket {
private:

public:
	Socket(std::string, std::string);
	~Socket(void);

	std::string read(void);
	void write(std::string);
};

#endif /* SOCKET_HPP */
