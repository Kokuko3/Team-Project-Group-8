import hashlib
import subprocess
import os

AES_KEY = "1234567890abcdef"
AES_IV = "abcdef1234567890"
HASH_FILE = "encrypted/hashes.txt"  

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

def load_expected_hashes(hash_file):
    """Loads expected hashes from the saved file."""
    hashes = {}
    if os.path.exists(hash_file):
        with open(hash_file, "r") as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) == 2:
                    filename, hash_val = parts
                    hashes[filename] = hash_val
    else:
        print(f"Error: Hash file '{hash_file}' not found!")
    return hashes

def verify_and_decrypt(received_folder, decrypted_folder, hash_file):
    """Verifies hash of received encrypted file, then decrypts."""
    expected_hashes = load_expected_hashes(hash_file)  # Load hashes dynamically

    os.makedirs(decrypted_folder, exist_ok=True)  

    for filename in os.listdir(received_folder):
        if filename.endswith(".enc"):
            encrypted_path = os.path.join(received_folder, filename)
            decrypted_path = os.path.join(decrypted_folder, filename.replace(".enc", ""))

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
    verify_and_decrypt("./received", "./decrypted", HASH_FILE)

