#!/bin/bash

./build/src/shttps --logtostderr=1 --port 5000 --document_root=./ --timeout=10 --v=2 --interface=ens3
