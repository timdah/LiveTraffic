/// @file       LTFlightRadar.h
/// @brief      FlightRadar24: Requests and processes live tracking data
/// @see        https://www.flightradar24.com/
/// @author     Tim Dahlmanns
/// @copyright  (c) 2024-2024 Tim Dahlmanns
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

#ifndef LTFlightRadar_h
#define LTFlightRadar_h

#include "LTChannel.h"

#define FR_CHECK_NAME        "flightradar24"
#define FR_CHECK_URL         "https://flighradar24.com"
#define FR_CHECK_POPUP       "Check flighradar24 coverage"

#define FR_NAME             "flightradar24"
#define FR_SLUG_BASE        "https://data-live.flightradar24.com/clickhandler/?flight=%06lx"
#define FR_URL              "https://data-cloud.flightradar24.com/zones/fcgi/feed.js?bounds=%f,%f,%f,%f"

constexpr int FR_TRANSP_ICAO   = 0;               // icao24
constexpr int FR_LAT           = 1;               // latitude
constexpr int FR_LON           = 2;               // longitude
constexpr int FR_HEADING       = 3;               // heading
constexpr int FR_CALC_ALT      = 4;               // calibrated altitude [ft]
constexpr int FR_SPD           = 5;               // ground speed
constexpr int FR_FEEDER        = 7;               // feeder station
constexpr int FR_AC_TYPE       = 8;               // aircraft type
constexpr int FR_REGISTRATION  = 9;               // registration
constexpr int FR_POS_TIME      = 10;              // position timestamp
constexpr int FR_ORIGIN        = 11;              // origin
constexpr int FR_DESTINATION   = 12;              // destination
constexpr int FR_FLIGHT_NR     = 13;              // flight number
constexpr int FR_VERT_SPD      = 15;              // vertical speed
constexpr int FR_CALL          = 16;              // callsign
constexpr int FR_AIRLINE       = 18;              // airline
                                                  
class FlightRadarConnection : public LTFlightDataChannel
{
public:
    FlightRadarConnection ();                                    ///< Constructor
    std::string GetURL (const positionTy& pos) override;    ///< Compile FlightRadar24 request URL

protected:
    void Main () override;                                  ///< virtual thread main function
    bool ProcessFetchedData () override;                    /// Process FlightRadar24 data format
    // bool ProcessErrors (const JSON_Object*) override        ///< No specific error processing for FlightRadar24
    // { return true; }
};


#endif /* LTFlightRadar_h */
