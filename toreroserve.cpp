/**
 * ToreroServe: A Lean Web Server
 * COMP 375 (Spring 2018) Project 02
 *
 * This program should take two arguments:
 * 	1. The port number on which to bind and listen for connections
 * 	2. The directory out of which to serve files.
 *
 * Author 1: Om Kanwar okanwar@sandiego.edu
 * Author 2: Chris Jung christopherjung@sandiego.edu
 */

// standard C libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

// operating system specific libraries
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

// C++ standard libraries
#include <vector>
#include <thread>
#include <string>
#include <iostream>
#include <system_error>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/lexical_cast.hpp>
#include <regex>
#include <ctime>
#include <fstream>
#include <iostream>
namespace fs = boost::filesystem;

using std::cout;
using std::string;
using std::vector;
using std::thread;

// This will limit how many clients can be waiting for a connection.
static const int BACKLOG = 10;
static const int bufferSize = 2048;

// forward declarations
int createSocketAndListen(const int port_num);
void acceptConnections(const int server_sock);
void handleClient(const int client_sock);
void sendData(int socked_fd, const char *data, size_t data_length);
int receiveData(int socked_fd, char *dest, size_t buff_size);
std::string dateToString(void);
void sendFileNotFound (const int client_sock, std::string httpTypeResponse);
//void sendOK (const int client_sock, int size, fs::path extension, std::vector<char> s);
void sendBadRequest (const int client_sock);

int main(int argc, char** argv) {

	/* Make sure the user called our program correctly. */
	if (argc != 3) {
		// TODO: print a proper error message informing user of proper usage
		cout << "INCORRECT USAGE!\n";
		exit(1);
	}
    /* Read the port number from the first command line argument. */
    int port = std::stoi(argv[1]);

	/* Create a socket and start listening for new connections on the
	 * specified port. */
	int server_sock = createSocketAndListen(port);
	
	/* Now let's start accepting connections. */
	acceptConnections(server_sock);

    close(server_sock);

	return 0;
}

/**
 * Sends message over given socket, raising an exception if there was a problem
 * sending.
 *
 * @param socket_fd The socket to send data over.
 * @param data The data to send.
 * @param data_length Number of bytes of data to send.
 */
void sendData(int socked_fd, const char *data, size_t data_length) {
	// TODO: Wrap the following code in a loop so that it keeps sending until
	// the data has been completely sent.
	
	int num_bytes_sent = send(socked_fd, data, data_length, 0);
	if (num_bytes_sent == -1) {
		std::error_code ec(errno, std::generic_category());
		throw std::system_error(ec, "send failed");
	}
}

/**
 * Receives message over given socket, raising an exception if there was an
 * error in receiving.
 *
 * @param socket_fd The socket to send data over.
 * @param dest The buffer where we will store the received data.
 * @param buff_size Number of bytes in the buffer.
 * @return The number of bytes received and written to the destination buffer.
 */
int receiveData(int socked_fd, char *dest, size_t buff_size) {
	int num_bytes_received = recv(socked_fd, dest, buff_size, 0);
	if (num_bytes_received == -1) {
		std::error_code ec(errno, std::generic_category());
		throw std::system_error(ec, "recv failed");
	}

	return num_bytes_received;
}

std::string dateToString ()
{
	time_t currentTime = time(0);
	char getDate[80];
	strftime(getDate, 80, "%a, %d %b %Y %X", localtime(&currentTime));
	std::string date_string(getDate);
	return date_string;
}

void sendFileNotFound (const int client_sock, std::string httpTypeResponse)
{
	// Create 404 Return Message
	std::string toReturn;
	toReturn += httpTypeResponse;
	toReturn.append(" 404 Not Found\r\nConnection: close\r\nDate: ");

	// Insert Date and Entity Body
	toReturn.append(dateToString());
	toReturn.append("\r\n\r\n");
	toReturn.append("<html><head><title>Page Not Found</title></head><body>404 Not Found</body></html>");
	cout << "Message is\r\n\r\n" << toReturn << '\n';

	// Copy to char array and send
	char message [toReturn.length() + 1];
	strcpy(message, toReturn.c_str());
	sendData(client_sock, message, sizeof(message)); 
}

void sendBadRequest (const int client_sock)
{
	// Create 400 Return Message
	std::string toReturn ("HTTP:/1.1 400 Bad Request\r\nConnection: close\r\nDate: ");
	
	// Insert Date 
	toReturn.append(dateToString());
	toReturn.append("\r\n");
	cout << "Message is\r\n\r\n" << toReturn << '\n';

	// Copy to char array and send
	char message [toReturn.length() + 1];
	strcpy(message, toReturn.c_str());
	sendData(client_sock, message, sizeof(message)); 
}

/*std::string generateStringFromFile (fs::path p)
{
	std::ifstream inFile;
	inFile.open(p.string(), std::ios::binary|std::ios::in);
	if (!inFile) 
	{
		cout << "Unable to open file\r\n";
	}
	inFile.seekg(0, std::ios::end);			
	std::streampos position = inFile.tellg();
	//cout << "length :" << position << "\r\n";
	inFile.seekg(0, std::ios::beg);
	//std::vector<char> buffer (position);
	std::vector<char> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
	int passPosition = (int)position;
	inFile.close();

	std::string entityBody(buffer.begin(), buffer.end());
//	std::copy(buffer.begin(), buffer.end(), entityBody);
	return entityBody;
}*/
void sendOK (const int client_sock, int size, fs::path extension, std::vector<char> s, std::string content)
{
	cout<< "got here\r\n";
	// Create 200 Return Message
	std::string toReturn ("HTTP/1.1 200 OK\r\nDate: ");

	cout << extension << "\r\n";
	// Insert Date, Size, Filetype
	toReturn.append(dateToString());
	toReturn.append("\r\n");
	toReturn.append("Content-Length: ");
	if (size < 0)
		toReturn.append(boost::lexical_cast<std::string>(content.length()));
	else
		toReturn.append(boost::lexical_cast<std::string>(s.size()));
	toReturn.append("\r\n");
	toReturn.append("Content-Type: ");
	std::string extension_string(extension.string());
	toReturn.append(extension_string.substr(1));
	toReturn.append("\r\n\r\n");
	cout << "here\r\n"; 
	// DON'T TOUCH THIS I DONT KNOW HOW IT WORKS 
	//int  messageSize = toReturn.length()+ 2 + strlen(content);
	int  messageSize = toReturn.length()+ 2 + size;
	if (size < 0)
		 messageSize = toReturn.length() + 2 + content.length();

	// Create 2 temporary buffers to hold message/entityBody
	char message [toReturn.length()+1];
	strcpy (message, toReturn.c_str());
	
	// Create final message, attach both and send
	char finalMessage[messageSize];
	memcpy(finalMessage, message, toReturn.length());
	
	if (size < 0)
	{
		char contentMessage[content.length() +1];
		strcpy(contentMessage, content.c_str());
		memcpy((finalMessage + toReturn.length()), contentMessage, content.length());
		sendData(client_sock, finalMessage, messageSize);
		cout << finalMessage << "\r\n";
		return;
	}

	char entityBody[s.size() + 1];
	std::copy(s.begin(), s.end(), entityBody);
	memcpy((finalMessage + toReturn.length()), entityBody, s.size());
	//memcpy((finalMessage + toReturn.length()), content, strlen(content));
	sendData(client_sock, finalMessage, messageSize); 	
	cout << finalMessage << "\r\n";
}

std::string generateIndexHTML(fs::path directory)
{
	std::vector<fs::directory_entry> list; 
	std::copy(fs::directory_iterator(directory), fs::directory_iterator(), std::back_inserter(list));
	std::string returnHTML ("<html><head><title>Parent Directory</title></head><body>Files under ");
	returnHTML.append(directory.string().substr(directory.string().find("/")));
	returnHTML.append("<br>");

	for (fs::directory_entry d : list)
	{	
		std::string nextLink ("<a href=\"");
		std::string pathString(d.path().string());
		std::size_t location = pathString.find("/");
		std::string pathSub(pathString.substr(location));
		nextLink.append(pathSub);
		nextLink.append("/\">");
		nextLink.append(pathSub);
		nextLink.append("/</a><br>");
		returnHTML.append(nextLink);
		//cout << d << "\r\n";
	}
	returnHTML.append("</body></html>");
	
	return returnHTML;
}

int containsIndex(fs::path directory)
{
	std::vector<fs::directory_entry> searchForIndex; 
	std::copy(fs::directory_iterator(directory), fs::directory_iterator(), std::back_inserter(searchForIndex));
	for (fs::directory_entry d : searchForIndex)
	{
		if (d.path().string().find("index.html") != std::string::npos)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * Receives a request from a connected HTTP client and sends back the
 * appropriate response.
 *
 * @note After this function returns, client_sock will have been closed (i.e.
 * may not be used again).
 *
 * @param client_sock The client's socket file descriptor.
 */
void handleClient(const int client_sock) {
	
	
	// TODO: Receive the request from the client. You can use receiveData here.
	char response_buffer[bufferSize];
	int response = receiveData(client_sock, response_buffer, sizeof(response_buffer));
	if (response <= 0)
	{
		return; //lol
		//There was no data received
	}
	cout << response_buffer<< "\n";	
		
	// TODO: Parse the request to determine what response to generate. I
	// recommend using regular expressions (specifically C++'s std::regex) to
	// determine if a request is properly formatted.
	std::regex regularExpression ("GET /.* HTTP/.*");
	if (!regex_match(response_buffer, regularExpression))
	{	
		sendBadRequest(client_sock);
		close(client_sock);
		return;
		// This means the request is not properly formatted (first line)
	}

	char temporary_buffer[bufferSize];
	std::copy(response_buffer, response_buffer+bufferSize, temporary_buffer);
	char * command = std::strtok(temporary_buffer, " ");
	char * location = std::strtok(NULL, " ");
	char * httpType = std::strtok(NULL, "\r");
	std::string location_string(location);
	std::string command_string(command);
	std::string httpType_string(httpType); 
	cout << "Command = <" << command_string << ">, Location = <"  << location_string << ">, HttpType = <" << httpType_string << ">\n";
		
	// TODO: Generate appropriate response.
	char search_buffer [512]; 
	std::string folder ("WWW");
	folder = folder + location_string;
	folder.copy(search_buffer, bufferSize);
	//strcat(search_buffer, location);
	//location_string.(search_buffer, bufferSize);

	cout << "Before send, send_buffer is :" << folder << "\n";
	fs::path p(folder);
	cout << p;

	// TODO: Generate response from file if exists
	if (fs::exists(p))
	{
		cout << p << " exists on server\n";
		int indexExists = 0;
		if (fs::is_directory(p))
		{	
			cout << p << " is directory\n";
			if (containsIndex(p) == 1)
			{
				std::string newPath(p.string());
				if (!newPath[newPath.length()-1] != '/')
					newPath.append("/");
				newPath.append("index.html");
				p = newPath;
				cout << "changed path to:" << p.string() << "\r\n";
			}
			else
			{
				std::string html;
				html = generateIndexHTML(p);
				cout << "abouttopass\r\n";
				sendOK(client_sock, -1, ".html", std::vector<char>(), html);
			}
		}
		if (fs::is_regular_file(p))
		{
			cout << p << " is regularFile\n";
			fs::path d(fs::extension(p));
			cout << fs::file_size(p) <<"\n";

			// char * file = generateBufferFromFile(p);
			// Read binary of file to string

			
			std::ifstream inFile;
			inFile.open(p.string(), std::ios::binary|std::ios::in);
			if (!inFile) 
			{
				cout << "Unable to open file\r\n";
			}
			inFile.seekg(0, std::ios::end);			
			std::streampos position = inFile.tellg();
			cout << "length :" << position << "\r\n";
			inFile.seekg(0, std::ios::beg);
			//std::vector<char> buffer (position);
			std::vector<char> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
			int passPosition = (int)position;
			inFile.close();
			
			// Append file to 200 OK Message
			sendOK(client_sock, passPosition, fs::extension(p), buffer, std::string());
			//sendOK(client_sock, -1, d, std::vector<char>(), generateStringFromFile(p));
			//sendOK(client_sock, 0, p, file);
		}	
	}
	else 
	{
		sendFileNotFound(client_sock, httpType_string);
		close(client_sock);
		return;
	}
	

	// TODO: Send response to client.


	
	// TODO: Close connection with client.
	memset(temporary_buffer, 0, sizeof(temporary_buffer));
	memset(search_buffer, 0, sizeof(search_buffer));
	memset(response_buffer, 0, sizeof(response_buffer));
}

/**
 * Creates a new socket and starts listening on that socket for new
 * connections.
 *
 * @param port_num The port number on which to listen for connections.
 * @returns The socket file descriptor
 */
int createSocketAndListen(const int port_num) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Creating socket failed");
        exit(1);
    }
    /* 
	 * A server socket is bound to a port, which it will listen on for incoming
     * connections.  By default, when a bound socket is closed, the OS waits a
     * couple of minutes before allowing the port to be re-used.  This is
     * inconvenient when you're developing an application, since it means that
     * you have to wait a minute or two after you run to try things again, so
     * we can disable the wait time by setting a socket option called
     * SO_REUSEADDR, which tells the OS that we want to be able to immediately
     * re-bind to that same port. See the socket(7) man page ("man 7 socket")
     * and setsockopt(2) pages for more details about socket options.
	 */
    int reuse_true = 1;

	int retval; // for checking return values

    retval = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_true,
                        sizeof(reuse_true));

    if (retval < 0) {
        perror("Setting socket option failed");
        exit(1);
    }

    /*
	 * Create an address structure.  This is very similar to what we saw on the
     * client side, only this time, we're not telling the OS where to connect,
     * we're telling it to bind to a particular address and port to receive
     * incoming connections.  Like the client side, we must use htons() to put
     * the port number in network byte order.  When specifying the IP address,
     * we use a special constant, INADDR_ANY, which tells the OS to bind to all
     * of the system's addresses.  If your machine has multiple network
     * interfaces, and you only wanted to accept connections from one of them,
     * you could supply the address of the interface you wanted to use here.
	 */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* 
	 * As its name implies, this system call asks the OS to bind the socket to
     * address and port specified above.
	 */
    retval = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (retval < 0) {
        perror("Error binding to port");
        exit(1);
    }

    /* 
	 * Now that we've bound to an address and port, we tell the OS that we're
     * ready to start listening for client connections. This effectively
	 * activates the server socket. BACKLOG (a global constant defined above)
	 * tells the OS how much space to reserve for incoming connections that have
	 * not yet been accepted.
	 */
    retval = listen(sock, BACKLOG);
    if (retval < 0) {
        perror("Error listening for connections");
        exit(1);
    }

	return sock;
}

/**
 * Sit around forever accepting new connections from client.
 *
 * @param server_sock The socket used by the server.
 */
void acceptConnections(const int server_sock) {
    while (true) {
        // Declare a socket for the client connection.
        int sock;

        /* 
		 * Another address structure.  This time, the system will automatically
         * fill it in, when we accept a connection, to tell us where the
         * connection came from.
		 */
        struct sockaddr_in remote_addr;
        unsigned int socklen = sizeof(remote_addr); 
		cout << "Start spinning\n";
        /* 
		 * Accept the first waiting connection from the server socket and
         * populate the address information.  The result (sock) is a socket
         * descriptor for the conversation with the newly connected client.  If
         * there are no pending connections in the back log, this function will
         * block indefinitely while waiting for a client connection to be made.
         */
        sock = accept(server_sock, (struct sockaddr*) &remote_addr, &socklen);
        if (sock < 0) {
            perror("Error accepting connection");
            exit(1);
        }
		cout << "found something\n";
        /* 
		 * At this point, you have a connected socket (named sock) that you can
         * use to send() and recv(). The handleClient function should handle all
		 * of the sending and receiving to/from the client.
		 *
		 * TODO: You shouldn't call handleClient directly here. Instead it
		 * should be called from a separate thread. You'll just need to put sock
		 * in a shared buffer and notify the threads (via a condition variable)
		 * that there is a new item on this buffer.
		 */
		handleClient(sock);

        /* 
		 * Tell the OS to clean up the resources associated with that client
         * connection, now that we're done with it.
		 */
        close(sock);
    }
}
