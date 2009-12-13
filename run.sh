#!/bin/bash

twait="80s"
port_udp=1126
port_tcp=1125
address="127.0.0.1"

# Executa o servidor
echo -n "Rodando o servidor... "
./bin/servidor $port_tcp $port_udp >/dev/null &
echo "done"

# Executa clientes tcp e udp
echo -n "Rodando os clientes... "
for i in `seq 2`; do
    ./bin/cliente $address $port_tcp "tcp" <./"intcp"$i.txt >./"outtcp"$i.txt &
    ./bin/cliente $address $port_udp "udp" <./"inudp"$i.txt >./"outudp"$i.txt &
done
echo "done"

echo -n "Esperando $twait para que os clientes udp terminem... "
sleep $twait
echo "done"

# Compara as saidas obtidas
for i in `seq 2`; do
    echo "Execucao loop $i: "

    temp=`diff "intcp"$i.txt "outtcp"$i.txt`
    if [ $? != 0 ]; then
        echo " * tcp: falhou"
    else
        echo " * tcp: ok"
    fi
    temp=`diff "inudp"$i.txt "outudp"$i.txt`
    if [ $? != 0 ]; then
        echo " * udp: falhou"
    else
        echo " * udp: ok"
    fi

    echo
done

# Mata o servidor
killall servidor &>/dev/null
