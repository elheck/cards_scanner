#!/bin/bash
set -e

# Directory for virtual environment
VENV_DIR="venv"

# Function to activate virtual environment
activate_venv() {
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
}

# Function to install Ansible
install_ansible() {
  # Upgrade pip and install Ansible
  echo "Upgrading pip and installing Ansible..."
  pip install --upgrade pip
  pip install ansible
}

# Function to install ansible-lint
install_ansible_lint() {
  # Upgrade pip and install ansible-lint
  echo "Upgrading pip and installing ansible-lint..."
  pip install --upgrade pip
  pip install ansible-lint
}

# Function to run ansible-lint on all playbooks
run_ansible_lint() {
  # Define the directory containing your playbooks
  PLAYBOOK_DIR="ansible"

  # Check if the directory exists
  if [ ! -d "$PLAYBOOK_DIR" ]; then
    echo "Directory $PLAYBOOK_DIR does not exist."
    exit 1
  fi

  # Find all YAML files in the directory
  playbooks=$(find "$PLAYBOOK_DIR" -type f -name "*.yml" -o -name "*.yaml")

  # Check if any playbooks were found
  if [ -z "$playbooks" ]; then
    echo "No playbooks found in $PLAYBOOK_DIR."
    exit 0
  fi

  # Run ansible-lint on each playbook
  for playbook in $playbooks; do
    echo "Linting $playbook..."
    ansible-lint "$playbook"
  done
}

# Function to run the top-level playbook
run_playbook() {
  # Execute the top-level playbook
  echo "Running top-level playbook..."
  ansible-playbook -i ansible/inventory.ini ansible/set_up_raspi.yml
}

# Main script execution
if [ "$1" == "lint" ]; then
  activate_venv
  install_ansible_lint
  run_ansible_lint
else
  activate_venv
  install_ansible
  run_playbook
fi

# (Optional) Deactivate the virtual environment when done
deactivate
