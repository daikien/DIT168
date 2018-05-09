#include "V2VService.hpp"

int main(int argc, char **argv) {
    std::shared_ptr<V2VService> v2vService = std::make_shared<V2VService>();

    //Authors: Simon Löfving, Filip Walldén and Boyan Dai

    //Get the commandline arguments
    //offset is to adjust the speed difference
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);

    //Check if they are provided
    if (commandlineArguments.count("ip") == 0 || commandlineArguments.count("tofollow") == 0 || commandlineArguments.count("offset") == 0 || commandlineArguments.count("delay") == 0)
    {
        std::cerr << "The car's IP, what group to follow, offset delay must be specified" << std::endl;
        return -1;
    }
    else
    { //Store the commandline arguments in declared variables
      OUR_IP = commandlineArguments["ip"];
      LEADER_ID = commandlineArguments["tofollow"];
      OFFSET = stof(commandlineArguments["offset"]);
      DELAY = stoi(commandlineArguments["delay"]);

      //Listening to interal calls
      od4 =
      std::make_shared<cluon::OD4Session>(214,
        [](cluon::data::Envelope &&envelope) noexcept {
          if(envelope.dataType() == 1041){
            opendlv::proxy::PedalPositionReading speed = cluon::extractMessage<opendlv::proxy::PedalPositionReading>(std::move(envelope));
            SPEED = speed.percent();
          }
          else if(envelope.dataType() == 1045){
            opendlv::proxy::GroundSteeringReading steering  = cluon::extractMessage<opendlv::proxy::GroundSteeringReading>(std::move(envelope));
            STEERING = steering.steeringAngle();
          }
      });

      //Loop with a frequency of 5 Herts. The announce presence will be called until another car is found, same goes for followRequest.
      //When a connection is established either followerStatus or leaderStatus will be called depending on if we lead or follow.
      auto loop{[&v2vService]() -> bool {
        v2vService->announcePresence();
        v2vService->followRequest(v2vService->presentCars[LEADER_ID]);
        v2vService->followerStatus();
        v2vService->leaderStatus(SPEED, STEERING, 0);

        return true;
      }};

       od4->timeTrigger(5, loop);

    }
}

/**
 * Implementation of the V2VService class as declared in V2VService.hpp
 */
V2VService::V2VService() {
    /*
     * The broadcast field contains a reference to the broadcast channel which is an OD4Session. This is where
     * AnnouncePresence messages will be received.
     */
    broadcast =
        std::make_shared<cluon::OD4Session>(BROADCAST_CHANNEL,
          [this](cluon::data::Envelope &&envelope) noexcept {
              std::cout << "[OD4] ";
              switch (envelope.dataType()) {
                  case ANNOUNCE_PRESENCE: {
                      AnnouncePresence ap = cluon::extractMessage<AnnouncePresence>(std::move(envelope));
                      std::cout << "received 'AnnouncePresence' from '"
                                << ap.vehicleIp() << "', GroupID '"
                                << ap.groupId() << "'!" << std::endl;

                      presentCars[ap.groupId()] = ap.vehicleIp();
                      od4->send(ap);
                      break;
                  }
                  default: std::cout << "Wrong channel dummy!" << std::endl;
              }
          });


    /*
     * Each car declares an incoming UDPReceiver for messages directed at them specifically. This is where messages
     * such as FollowRequest, FollowResponse, StopFollow, etc. are received.
     */
    incoming =
        std::make_shared<cluon::UDPReceiver>("0.0.0.0", DEFAULT_PORT,
           [this](std::string &&data, std::string &&sender, std::chrono::system_clock::time_point &&ts) noexcept {
               std::cout << "[UDP] ";
               std::pair<int16_t, std::string> msg = extract(data);

               switch (msg.first) {
                   case FOLLOW_REQUEST: {
                       FollowRequest followRequest = decode<FollowRequest>(msg.second);
                       std::cout << "received '" << followRequest.LongName()
                                 << "' from '" << sender << "'!" << std::endl;

                       // After receiving a FollowRequest, check first if there is currently no car already following.
                       if (followerIp.empty()) {
                           unsigned long len = sender.find(':');    // If no, add the requester to known follower slot
                           followerIp = sender.substr(0, len);      // and establish a sending channel.
                           toFollower = std::make_shared<cluon::UDPSender>(followerIp, DEFAULT_PORT);
                           followResponse();
                           od4->send(followRequest);
                       }
                       break;
                   }
                   case FOLLOW_RESPONSE: {
                       FollowResponse followResponse = decode<FollowResponse>(msg.second);
                       std::cout << "received '" << followResponse.LongName()
                                 << "' from '" << sender << "'!" << std::endl;
                       od4->send(followResponse);
                       break;
                   }
                   case STOP_FOLLOW: {
                       StopFollow stopFollow = decode<StopFollow>(msg.second);
                       std::cout << "received '" << stopFollow.LongName()
                                 << "' from '" << sender << "'!" << std::endl;

                       // Clear either follower or leader slot, depending on current role.
                       unsigned long len = sender.find(':');
                       if (sender.substr(0, len) == followerIp) {
                           followerIp = "";
                           toFollower.reset();
                       }
                       else if (sender.substr(0, len) == leaderIp) {
                           leaderIp = "";
                           toLeader.reset();
                       }
                       od4->send(stopFollow);
                       break;
                   }
                   case FOLLOWER_STATUS: {
                       FollowerStatus followerStatus = decode<FollowerStatus>(msg.second);
                       std::cout << "received '" << followerStatus.LongName()
                                 << "' from '" << sender << "'! " << std::endl;

                       od4->send(followerStatus);
                       break;
                   }
                   case LEADER_STATUS: {
                       LeaderStatus leaderStatus = decode<LeaderStatus>(msg.second);
                       std::cout << "Received " << leaderStatus.LongName()
                                 << " from " << sender
                                 << " Speed: " << leaderStatus.speed()
                                 << " Angle: " << leaderStatus.steeringAngle()
                       << std::endl;
                    // Steering delay. Authors: Simon Löfving and Boyan Dai
                    // In order to get accurate steering instruction when we recieved leaderstatus from another car,
                    // We check the cmdQ against the DELAY variable which was intiliazed in the commandline.
                    // The speed shouldnt be delayed and therefore it is sent in every if else statement.
                           opendlv::proxy::PedalPositionReading pedal;
                           opendlv::proxy::GroundSteeringReading steer;
                           SPEED = leaderStatus.speed();
                    // If the leading car is going forward or backwards and there are more units of commands in the queue
                    // than the specified DELAY we push the leaders steeering command to the queue, sends the oldest command in the queue
                    // to our car and then erase the command from the queue.
                         if(SPEED != 0 && cmdQ.size() > DELAY){
                            cmdQ.push(leaderStatus.steeringAngle());
                            pedal.percent(SPEED + OFFSET);
                            od4->send(pedal);
                            steer.steeringAngle(cmdQ.front());
                            od4->send(steer);
                            cmdQ.pop();
                    // If the DELAY is larger than the size of the queue, we add 0 to the queue. Due to this we fill up the queue
                    // and get a steering delay when we follow.
                         }else if(SPEED != 0){
                             cmdQ.push(0);
                    // The OFFSET here is changable by changing the initial arguments, for avoiding the different performance due to the hardware difference
                             pedal.percent(SPEED + OFFSET);
                             od4->send(pedal);
                    // In any other case, for example no speed, were doing nothing.
                         }else {
                             pedal.percent(SPEED);
                             od4->send(pedal);
                             steer.steeringAngle(0);
                             od4->send(steer);
                         }
                        od4->send(leaderStatus);
                    break;
                   }
                   default: std::cout << "¯\\_(ツ)_/¯" << std::endl;
               }
           });
}

/**
 * This function sends an AnnouncePresence (id = 1001) message on the broadcast channel. It will contain information
 * about the sending vehicle, including: IP, port and the group identifier.
 */
void V2VService::announcePresence() {
    if (!followerIp.empty() || !leaderIp.empty()) return;
    AnnouncePresence announcePresence;
    announcePresence.vehicleIp(OUR_IP);
    announcePresence.groupId(GROUP_ID );
    broadcast->send(announcePresence);
    od4->send(announcePresence);
}

/**
 * This function sends a FollowRequest (id = 1002) message to the IP address specified by the parameter vehicleIp. And
 * sets the current leaderIp field of the sending vehicle to that of the target of the request.
 *
 * @param vehicleIp - IP of the target for the FollowRequest
 */
void V2VService::followRequest(std::string vehicleIp) {
    if (!leaderIp.empty()) return;
    leaderIp = vehicleIp;
    toLeader = std::make_shared<cluon::UDPSender>(leaderIp, DEFAULT_PORT);
    FollowRequest followRequest;
    toLeader->send(encode(followRequest));
    od4->send(followRequest);
}

/**
 * This function send a FollowResponse (id = 1003) message and is sent in response to a FollowRequest (id = 1002).
 * This message will contain the NTP server IP for time synchronization between the target and the sender.
 */
void V2VService::followResponse() {
    if (followerIp.empty()) return;
    FollowResponse followResponse;
    toFollower->send(encode(followResponse));
    od4->send(followResponse);
}

/**
 * This function sends a StopFollow (id = 1004) request on the ip address of the parameter vehicleIp. If the IP address
 * is neither that of the follower nor the leader, this function ends without sending the request message.
 *
 * @param vehicleIp - IP of the target for the request
 */
void V2VService::stopFollow(std::string vehicleIp) {
    StopFollow stopFollow;
    if (vehicleIp == leaderIp) {
      toLeader->send(encode(stopFollow));
      leaderIp = "";
      toLeader.reset();
    }
    if (vehicleIp == followerIp) {
      toFollower->send(encode(stopFollow));
      followerIp = "";
      toFollower.reset();
    }
    od4->send(stopFollow);
}

/**
 * This function sends a FollowerStatus (id = 3001) message on the leader channel.
 */
void V2VService::followerStatus() {
  if (!leaderIp.empty()) {
    FollowerStatus followerStatus;
    toLeader->send(encode(followerStatus));
    od4->send(followerStatus);
  }
}

/**
 * This function sends a LeaderStatus (id = 2001) message on the follower channel.
 *
 * @param speed - current velocity
 * @param steeringAngle - current steering angle
 * @param distanceTraveled - distance traveled since last reading
 */
void V2VService::leaderStatus(float speed, float steering, uint8_t distanceTraveled) {
  if (!followerIp.empty()) {
    LeaderStatus leaderStatus;
    leaderStatus.timestamp(getTime());
    leaderStatus.speed(speed);
    leaderStatus.steeringAngle(steering);
    leaderStatus.distanceTraveled(distanceTraveled);
    toFollower->send(encode(leaderStatus));
    od4->send(leaderStatus);
  }
}


/**
 * Gets the current time.
 *
 * @return current time in milliseconds
 */
 uint64_t V2VService::getTime() {
     timeval now;
     gettimeofday(&now, nullptr);
     return (uint64_t ) now.tv_usec / 1000;
 }

/**
 * The extraction function is used to extract the message ID and message data into a pair.
 *
 * @param data - message data to extract header and data from
 * @return pair consisting of the message ID (extracted from the header) and the message data
 */
std::pair<int16_t, std::string> V2VService::extract(std::string data) {
    if (data.length() < 10) return std::pair<int16_t, std::string>(-1, "");
    int id, len;
    std::stringstream ssId(data.substr(0, 4));
    std::stringstream ssLen(data.substr(4, 10));
    ssId >> std::hex >> id;
    ssLen >> std::hex >> len;
    return std::pair<int16_t, std::string> (
            data.length() -10 == len ? id : -1,
            data.substr(10, data.length() -10)
    );
};

/**
 * Generic encode function used to encode a message before it is sent.
 *
 * @tparam T - generic message type
 * @param msg - message to encode
 * @return encoded message
 */
template <class T>
std::string V2VService::encode(T msg) {
    cluon::ToProtoVisitor v;
    msg.accept(v);
    std::stringstream buff;
    buff << std::hex << std::setfill('0')
         << std::setw(4) << msg.ID()
         << std::setw(6) << v.encodedData().length()
         << v.encodedData();
    return buff.str();
}

/**
 * Generic decode function used to decode an incoming message.
 *
 * @tparam T - generic message type
 * @param data - encoded message data
 * @return decoded message
 */
template <class T>
T V2VService::decode(std::string data) {
    std::stringstream buff(data);
    cluon::FromProtoVisitor v;
    v.decodeFrom(buff);
    T tmp = T();
    tmp.accept(v);
    return tmp;
}
