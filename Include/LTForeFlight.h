/// @file       LTForeFlight.h
/// @brief      ForeFlight: Output channel to send LiveTraffic's aircraft positions to the local network
/// @see        https://www.foreflight.com/support/network-gps/
/// @see        https://www.foreflight.com/connect/spec/
///             for the address discovery protocol via broadcast
/// @details    Starts/stops a separate thread to
///             - listen for a ForeFlight client to send its address
///             - then send flight data to that address as UDP unicast
/// @details    Starts/stops a separate thread to send out UDP broadcast.\n
///             Formats and sends UDP packages.\n
/// @author     Birger Hoppe
/// @copyright  (c) 2018-2020 Birger Hoppe
/// @copyright  Permission is hereby granted, free of charge, to any person obtaining a
///             copy of this software and associated documentation files (the "Software"),
///             to deal in the Software without restriction, including without limitation
///             the rights to use, copy, modify, merge, publish, distribute, sublicense,
///             and/or sell copies of the Software, and to permit persons to whom the
///             Software is furnished to do so, subject to the following conditions:\n
///             The above copyright notice and this permission notice shall be included in
///             all copies or substantial portions of the Software.\n
///             THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///             IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///             FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///             AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///             LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///             OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
///             THE SOFTWARE.

#ifndef LTForeFlight_h
#define LTForeFlight_h

#include "LTChannel.h"

//
// MARK: ForeFlight Constants
//

#define FF_CHECK_NAME           "ForeFlight Mobile EFB"
#define FF_CHECK_URL            "https://foreflight.com/products/foreflight-mobile/"
#define FF_CHECK_POPUP          "Open ForeFlight's web site about the Mobile EFB"

#define FOREFLIGHT_NAME        "ForeFlight"
constexpr size_t FF_NET_BUF_SIZE    = 512;

// sending intervals in milliseonds
constexpr std::chrono::milliseconds FF_INTVL_GPS    = std::chrono::milliseconds(1000); // 1 Hz
constexpr std::chrono::milliseconds FF_INTVL_ATT    = std::chrono::milliseconds( 200); // 5 Hz
constexpr std::chrono::milliseconds FF_INTVL        = std::chrono::milliseconds(  20); // Interval between two

#define MSG_FF_LISTENING        "ForeFlight: Waiting for a ForeFlight device to broadcast its address..."
#define MSG_FF_SENDING          "ForeFlight: Starting to send to %s"
#define MSG_FF_NOT_SENDING      "ForeFlight: No longer sending to %s"
#define MSG_FF_STOPPED          "ForeFlight: Stopped"

//
// MARK: ForeFlight Sender
//
class ForeFlightSender : public LTOutputChannel
{
protected:
    /// State of the interface
    enum FFStateTy : int {
        FF_STATE_NONE = 0,              ///< Not doing anything
        FF_STATE_DISCOVERY,             ///< Waiting for a ForeFlight device to broadcast its address on the network
        FF_STATE_SENDING,               ///< Actually sending data to a discovered device
    } state = FF_STATE_NONE;
    std::string ffAddr;                 ///< Addresses of the ForeFlight apps we are sending to
    /// UDP sockets for sending UDP datagrams from/to ForeFlight apps
    std::map<XPMP2::SockAddrTy, XPMP2::UDPReceiver> mapUdp;

public:
    ForeFlightSender ();

    std::string GetURL (const positionTy&) override { return ""; }   // don't need URL, no request/reply
    
    // interface called from LTChannel
    bool FetchAllData(const positionTy&) override { return false; }
    bool ProcessFetchedData () override { return true; }
    std::string GetStatusText () const override;  ///< return a human-readable staus

protected:
    // send positions
    void Main () override;          ///< virtual thread main function
    void SendGPS (const positionTy& pos, double speed_m, double track); // position of user's aircraft
    void SendAtt (const positionTy& pos, double speed_m, double track); // attitude of user's aircraft
    void SendAllTraffic (); // other traffic
    void SendTraffic (const LTFlightData& fd);
};

#endif /* LTForeFlight_h */
