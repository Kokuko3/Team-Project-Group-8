import hashlib
import subprocess
import os
import shutil

AES_KEY = "1234567890abcdef"
AES_IV = "abcdef1234567890"
ENCRYPTED_FOLDER = "encrypted"
RECEIVED_FOLDER = "received"
DECRYPTED_FOLDER = "decrypted"
HASH_FILE = f"{ENCRYPTED_FOLDER}/hashes.txt"

def setup_folders():
    """Ensure required directories exist and move files if needed."""
    os.makedirs(RECEIVED_FOLDER, exist_ok=True)
    os.makedirs(DECRYPTED_FOLDER, exist_ok=True)

    # Move encrypted files if not already in received/
    if not os.listdir(RECEIVED_FOLDER):  
        for file in os.listdir(ENCRYPTED_FOLDER):
            if file.endswith(".enc") or file == "hashes.txt":
                shutil.copy(os.path.join(ENCRYPTED_FOLDER, file), RECEIVED_FOLDER)

def compute_md5(file_path):
    """Computes MD5 hash of a file."""
    hasher = hashlib.md5()
    with open(file_path, "rb") as f:
        hasher.update(f.read())
    return hasher.hexdigest()

def decrypt_image(encrypted_path, decrypted_path):
    """Decrypts an image using AES-128-CBC."""
    cmd = f"openssl enc -d -aes-128-cbc -K {AES_KEY.encode().hex()} -iv {AES_IV.encode().hex()} -in {encrypted_path} -out {decrypted_path}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error: Decryption failed for {encrypted_path}. {result.stderr}")
        return False
    return True

def load_expected_hashes():
    """Loads expected hashes from the saved file."""
    hashes = {}
    if os.path.exists(HASH_FILE):
        with open(HASH_FILE, "r") as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) == 2:
                    filename, hash_val = parts
                    hashes[filename] = hash_val
    else:
        print(f"Error: Hash file '{HASH_FILE}' not found!")
    return hashes

def verify_and_decrypt():
    """Verifies hash of received encrypted file, then decrypts."""
    expected_hashes = load_expected_hashes()

    for filename in os.listdir(RECEIVED_FOLDER):
        if filename.endswith(".enc"):
            encrypted_path = os.path.join(RECEIVED_FOLDER, filename)
            decrypted_path = os.path.join(DECRYPTED_FOLDER, filename.replace(".enc", ""))

            # Compute received encrypted hash
            computed_enc_hash = compute_md5(encrypted_path)

            # Compare with expected hash
            if computed_enc_hash != expected_hashes.get(filename, ""):
                print(f"Error: Hash mismatch for {filename}. Requesting resend.")
                continue

            print(f"{filename}: Hash verified. Proceeding with decryption.")

            # Decrypt the file
            if decrypt_image(encrypted_path, decrypted_path):
                print(f"Decryption successful: {decrypted_path}")

if __name__ == "__main__":
    setup_folders()
    verify_and_decrypt()
