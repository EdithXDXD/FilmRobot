#!/bin/bash

echo "start sending to resberry"
curl -H "Content-Type: application/json" http://35.185.235.43:3000/handMotion -X POST  -d @data.txt