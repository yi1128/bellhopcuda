#!/bin/bash

echo "Eigenray:"
find ../bellhop/tests -name "*.env" | while read f; do
    res=$(sed -n '10,$s|^'\''E[^3/]*'\''|&|p' $f)
    if [ -n "$res" ]; then
        echo "$f: $res"
    fi
done

exit

echo "Arrivals:"
find ../bellhop/tests -name "*.env" | while read f; do
    res=$(sed -n '10,$s|^'\''[Aa][^3/]*'\''|&|p' $f)
    if [ -n "$res" ]; then
        echo "$f: $res"
    fi
done

echo "Ray:"
find ../bellhop/tests -name "*.env" | while read f; do
    res=$(sed -n '10,$s|^'\''R[^3/]*'\''|&|p' $f)
    if [ -n "$res" ]; then
        echo "$f: $res"
    fi
done

echo "Coherent:"
find ../bellhop/tests -name "*.env" | while read f; do
    res=$(sed -n '10,$s|^'\''C[CRSbBgG][^3/]*'\''|&|p' $f)
    if [ -n "$res" ]; then
        echo "$f: $res"
    fi
done

echo "Semi-coherent:"
find ../bellhop/tests -name "*.env" | while read f; do
    res=$(sed -n '10,$s|^'\''S[CRSbBgG][^3/]*'\''|&|p' $f)
    if [ -n "$res" ]; then
        echo "$f: $res"
    fi
done

echo "Incoherent:"
find ../bellhop/tests -name "*.env" | while read f; do
    res=$(sed -n '10,$s|^'\''I[CRSbBgG][^3/]*'\''|&|p' $f)
    if [ -n "$res" ]; then
        echo "$f: $res"
    fi
done
