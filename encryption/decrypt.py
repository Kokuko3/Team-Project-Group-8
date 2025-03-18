import hashlib
import subprocess
import os

AES_KEY = "1234567890abcdef"
AES_IV = "abcdef1234567890"   

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

def verify_and_decrypt(received_folder, decrypted_folder, expected_hashes):
    """Verifies hash of received encrypted file, then decrypts."""
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
    expected_hashes = {
        "image1.png.enc": "abc123",  # Populate with actual expected hashes
        "image2.png.enc": "def456",
    }
    verify_and_decrypt("./received", "./decrypted", expected_hashes)

