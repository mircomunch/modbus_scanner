#include <MQTT.hpp>

MQTT::MQTT(String board_id, char* host, uint16_t port)
{
	this->_board_id = board_id;
	this->_host = host;
	this->_port = port;
	this->_cert_en = false;
	this->client.setServer(this->_host, this->_port);
	this->client.setClient(this->_unsecuredClient);
}
MQTT::MQTT(String board_id, char* host, uint16_t port, int msg_size)
{
	this->_board_id = board_id;
	this->_host = host;
	this->_port = port;
	this->_msg_size = msg_size;
	this->_cert_en = false;
	this->client.setServer(this->_host, this->_port);
	this->client.setClient(this->_unsecuredClient);
	this->client.setBufferSize(this->_msg_size);
}
MQTT::MQTT(String board_id, char* host, uint16_t port, int msg_size, bool cert_en, char* user, char* psw)
{
	this->_board_id = board_id;
	this->_host = host;
	this->_port = port;
	this->_msg_size = msg_size;
	this->_cert_en = cert_en;
	this->_user = user;
	this->_psw = psw;
	this->client.setServer(this->_host, this->_port);
	this->client.setClient(this->_securedClient);
	if(_cert_en) {
		this->_securedClient.setCACert(root_ca);
	}
	this->client.setBufferSize(this->_msg_size);
}

bool MQTT::reconnect()
{
	Serial.print("Attempting MQTT connection… ");
	String clientId = this->_board_id;
	clientId += String(random(0xffff), HEX); // Create a random client ID

	// Attempt to connect
	if(this->_user && this->_psw) {
		Serial.println("authenticated");
		this->client.connect(clientId.c_str(), this->_user, this->_psw);
	}
	else {
		Serial.println("un-authenticated");
		this->client.connect(clientId.c_str());
	}
	return client.connected();
}

bool MQTT::publishMessage(const char *topic, String payload, boolean retained)
{
    return this->client.publish(topic, payload.c_str(), retained);
}