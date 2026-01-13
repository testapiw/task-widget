#!/bin/bash

start_services() {
    echo "Starting MySQL..."
    sudo systemctl start mysql

    echo "Starting Nginx and PHP-FPM..."
    sudo systemctl start nginx
    sudo systemctl start php8.5-fpm

    echo "All services started."
}

stop_services() {
    echo "Stopping Nginx and PHP-FPM..."
    sudo systemctl stop nginx
    sudo systemctl stop php8.5-fpm
    # sudo systemctl stop php7.4-fpm php8.4-fpm php8.2-fpm 2>/dev/null

    echo "Stopping MySQL..."
    # Attempt to stop MySQL with a 30-second timeout
    sudo timeout 30s systemctl stop mysql
    
    # Check if the command timed out (exit code 124)
    if [ $? -eq 124 ]; then
        echo "MySQL is stubborn. Forcing termination..."
        # Forcefully kill the process if it fails to stop gracefully
        sudo killall -9 mysqld
    fi

    echo "Services stopped."
}

case "$1" in
    start)
        start_services
        ;;
    stop)
        stop_services
        ;;
    *)
        echo "Usage: $0 start|stop"
        exit 1
        ;;
esac