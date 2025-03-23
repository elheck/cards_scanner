#!/bin/bash
set -e

# Directory for virtual environment
VENV_DIR="venv"

# Check if virtual environment exists; if not, create it
if [ ! -d "$VENV_DIR" ]; then
  echo "Virtual environment not found. Creating a new one..."
  python3 -m venv "$VENV_DIR"
else
  echo "Virtual environment already exists."
fi

# Activate the virtual environment
echo "Activating virtual environment..."
source "$VENV_DIR/bin/activate"

# Upgrade pip and install Ansible
echo "Upgrading pip and installing Ansible..."
pip install --upgrade pip
pip install ansible

# Execute the top-level playbook
echo "Running top-level playbook..."
ansible-playbook -i ansible/inventory.ini ansible/set_up_raspi.yml

# (Optional) Deactivate the virtual environment when done
deactivate
