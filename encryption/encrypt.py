import hashlib
import subprocess
import os

AES_KEY = "1234567890abcdef"  # 16-byte AES Key (hex format)
AES_IV = "abcdef1234567890"   # 16-byte AES IV (hex format)

def compute_md5(file_path):
    """Computes MD5 hash of a file."""
    hasher = hashlib.md5()
    with open(file_path, "rb") as f:
        hasher.update(f.read())
    return hasher.hexdigest()

def encrypt_image(image_path, encrypted_path):
    """Encrypts an image using AES-128-CBC."""
    cmd = f"openssl enc -aes-128-cbc -K {AES_KEY.encode().hex()} -iv {AES_IV.encode().hex()} -in {image_path} -out {encrypted_path}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error: Encryption failed for {image_path}. {result.stderr}")
        return None
    return encrypted_path

def process_images(image_folder, encrypted_folder):
    """Encrypts images, computes hash, and prints hash for verification."""
    for filename in os.listdir(image_folder):
        if filename.endswith(".png"):
            image_path = os.path.join(image_folder, filename)
            encrypted_path = os.path.join(encrypted_folder, filename + ".enc")

            encrypted_file = encrypt_image(image_path, encrypted_path)
            if encrypted_file:
                encrypted_hash = compute_md5(encrypted_file)
                print(f"{filename}.enc Hash: {encrypted_hash}")

if __name__ == "__main__":
    process_images("./images", "./encrypted")

