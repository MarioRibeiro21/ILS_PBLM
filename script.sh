#!/bin/bash

for num in {3..15}; do
    echo "Executando para num_maquinas = $num"
    for i in {1..20}; do
        ./codigo "$num"
        if [ $? -ne 0 ]; then
            echo "Erro ao executar o programa para num_maquinas = $num, iteracao $i"
        fi
    done
done