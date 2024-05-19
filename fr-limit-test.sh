#!/bin/bash

while true; do
  # curl -o /dev/null -s -w "%{http_code}\n" "https://data-cloud.flightradar24.com/zones/fcgi/feed.js?bounds=51.695934486820086,51.33620924004462,7.3188623489167215,7.8969423900388565"
  curl "https://data-cloud.flightradar24.com/zones/fcgi/feed.js?bounds=51.695934486820086,51.33620924004462,7.3188623489167215,7.8969423900388565"
  sleep 20
done
