#!/bin/bash

make debug;
sudo ./swiper "{echoLeftPin: 24, triggerLeftPin: 23, echoRightPin: 26, triggerRightPin: 25, threshold: 175, distanceDiff: 1.25, debug: false, delay: 100, sensorThrottling: true, maxDelay: 2000}"
