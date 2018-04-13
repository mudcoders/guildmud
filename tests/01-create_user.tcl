#!/usr/bin/env expect 

send_user "Running expect: $argv0\n" 

log_user 0

set user "testuser"
set password "testuser"
set server "localhost"
set port "9009"

# 1|Testuser|$2y$12$PnLEkiA888KDMlRcVDtql.BVvTKyJ.6y9epr3xUVoSbTGDCHNf/3y|2

exec sqlite3 ../data/guildmud.db "delete from players where name='Testuser';"

spawn telnet $server $port

expect "name? "
send "$user\r"
expect "password: "
send "$password\n"
expect "password: "
send "$password\n"

expect {
    "Welcome to SocketMUD(tm)" { 
        send "quit\r"
        send_user "100%: Checks: 1, Failures: 0, Errors: 0\n"
        exit 0
    }
    default {
        send_user "100%: Checks: 0, Failures: 1, Errors: 0\n"
        exit 1
    }
}