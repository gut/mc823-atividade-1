#!/bin/bash

port=1028

for i in `seq 0 10`; do
    echo "Testing backlog $i"
    # Executa o servidor com o backlog
    ./bin/servidor $port $i >/dev/null &

    # Executa 10 clientes "simultaneamente"
    for j in `seq 10`; do
        ./bin/cliente 127.0.0.1 $port >/dev/null &
    done

    # Salva estado das conexoes num arquivo
    netstat -t -a -n -p 2>/dev/null | egrep 'servidor|cliente' &>>backlog$i.txt
    echo >>backlog$i.txt
    # Salva numero de conexoes completadas no fim do arquivo
    clinum=`cat backlog$i.txt | egrep 'ESTABLISHED.*cliente' | wc -l`
    echo "Numero de clientes simultaneos: $clinum" >>backlog$i.txt

    echo "Saida do netstat salva em backlog$i.txt"

    # Mata os processos clientes e o servidor
    echo "Matando todos os  processos"
    killall cliente &>/dev/null
    killall servidor &>/dev/null

    sleep 10
    # Muda de porta para evitar erro "Address already in use"
    port=$((port+1))
    echo
done
