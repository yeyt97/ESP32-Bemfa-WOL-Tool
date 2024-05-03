#include <WiFi.h>
#include <map>
#include <utility>

const char* TCP_SERVER_ADDR = "bemfa.com";
const char* TCP_SERVER_PORT = "8344";
const int MAX_PACKET_SIZE = 512;
const int KEEP_ALIVE_TIME = 60 * 1000;  // 心跳维持周期

class BemfaClient {
private:
    WiFiClient tcpClient;
    String buff = "";
    unsigned long msgProcTick = 0;  // 消息处理计时
    unsigned long heartTick = 0; // 心跳计时
    unsigned long tcpStartTick = 0; // 上次连接的计时
    bool isConnected = false;

    const String uid;
    const String topic;

    std::map<std::string, std::function<void()>> commandMap;

public:
    BemfaClient(const char *UID, const char *TOPIC): uid(UID), topic(TOPIC) {}

    void addCommand(const std::string& key, std::function<void()> func) {
        commandMap[key] = std::move(func);
    }

private:
    void executeCommand(const std::string& key) {
        // 在映射中查找键对应的函数
        auto it = commandMap.find(key);
        if (it == commandMap.end()) {
            Serial.print("Command not found: ");
            Serial.println(key.c_str());
            return;
        }
        it->second();
    }

    /*
     * 发送数据到巴法云
     */
    void send(String p) {
        if (!tcpClient.connected()) {
            Serial.println("Client is not ready");
            return;
        }
        tcpClient.print(p);
        Serial.println(p);
        heartTick = millis();//心跳计时开始，需要每隔60秒发送一次数据
    }

    /*
     * 连接到巴法云服务器
     */
    void start() {
        if(tcpClient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT))){
            Serial.print("\nConnected to server:");
            Serial.printf("%s:%d\r\n",TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT));
            send("cmd=1&uid=" + uid + "&topic=" + topic + "\r\n");

            isConnected = true;
            tcpClient.setNoDelay(true);
        } else {
            Serial.print("Failed connected to server:");
            Serial.println(TCP_SERVER_ADDR);
            tcpClient.stop();
            isConnected = false;
        }
        tcpStartTick = millis();
    }

public:
    void tick(){
        if(WiFi.status() != WL_CONNECTED) {
            return;
        }
        if (!tcpClient.connected()) {
            if(isConnected){
                isConnected = false;
                tcpStartTick = millis();
                Serial.println("\r\nTCP Client disconnected.");
                tcpClient.stop();
            }
            else if(millis() - tcpStartTick > 1000)
                start();
        } else {
            if (tcpClient.available()) {
                char c = tcpClient.read();
                buff += c;
                msgProcTick = millis();
                // 如果消息满了直接处理一次
                if(buff.length() >= MAX_PACKET_SIZE - 1){
                    msgProcTick -= 200;
                }
                heartTick = millis();
            }
            // 心跳保活
            if(millis() - heartTick >= KEEP_ALIVE_TIME){
                heartTick = millis();
                Serial.println("--Keep alive:");
                send("cmd=0&msg=keep\r\n");
            }
        }
        // 200ms处理一次消息
        if((buff.length() >= 1) && (millis() - msgProcTick >= 200)) {
            processMsg();
        }
    }

    void processMsg() {
        tcpClient.flush();
        Serial.print("Rev string: ");
        buff.trim();
        Serial.println(buff);
        String rcv_topic = "";
        String rcv_msg = "";
        if(buff.length() > 15){
            int topicIndex = buff.indexOf("&topic=") + 7;
            int msgIndex = buff.indexOf("&msg=");
            rcv_topic = buff.substring(topicIndex, msgIndex);
            rcv_msg = buff.substring(msgIndex + 5);
            Serial.println("topic:------" + rcv_topic);
            Serial.println("msg:--------" + rcv_msg);
            executeCommand(rcv_msg.c_str());
        }
        buff = "";
    }
};