#!/bin/bash

sudo apt update
sudo apt install -y python3 python3-pip python3-venv

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

source venv/bin/activate

pip install --upgrade pip
pip install flask pyserial

echo "Done. To run:"
echo "  source venv/bin/activate"
echo "  python app.py"
