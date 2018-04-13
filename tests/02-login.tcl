#!/usr/bin/env expect 

send_user "Running expect: $argv0\n" 

log_user 0

set user "testuser"
set password "testuser"
set server "localhost"
set port "9009"


spawn telnet $server $port

expect "name? "
send "$user\r"
expect "password? "
send "$password\n"


expect {
    "You take over a body already in use." { 
        send "quit\r"
        send_user "100%: Checks: 1, Failures: 0, Errors: 0\n"
        exit 0
    }
    default {
        send_user "100%: Checks: 0, Failures: 1, Errors: 0\n"
        exit 1
    }
}
