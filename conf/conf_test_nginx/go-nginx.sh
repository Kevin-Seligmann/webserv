#!/bin/sh

LOCAL_CONF=$(pwd)/conf
LOCAL_HTML1=$(pwd)/html/server1
LOCAL_HTML2=$(pwd)/html/server2
LOCAL_HTML3=$(pwd)/html/server3
CONTAINER_NAME="nginx-test"
IMAGE="nginx:stable"

docker stop $CONTAINER_NAME 2>/dev/null
docker rm $CONTAINER_NAME 2>/dev/null

docker run -d -p 8080:80 -p 8081:81 -p 8082:82 \
  -v $LOCAL_CONF:/etc/nginx/conf.d \
  -v $LOCAL_HTML1:/usr/share/nginx/html/server1 \
  -v $LOCAL_HTML2:/usr/share/nginx/html/server2 \
  -v $LOCAL_HTML3:/usr/share/nginx/html/server3 \
  --name $CONTAINER_NAME $IMAGE

