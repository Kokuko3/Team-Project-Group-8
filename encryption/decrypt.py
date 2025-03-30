import subprocess
import os

AES_KEY = "1234567890abcdef"  # 16-byte AES Key
AES_IV = "abcdef1234567890"   # 16-byte AES IV

def decrypt_image(input_path, output_path):
    """Decrypts an image using AES-128-CBC."""
    cmd = f"openssl enc -d -aes-128-cbc -K {AES_KEY.encode().hex()} -iv {AES_IV.encode().hex()} -in {input_path} -out {output_path}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error decrypting {input_path}: {result.stderr}")
    else:
        print(f"Decrypted: {output_path}")

def decrypt_all_images(input_folder, output_folder):
    """Decrypts all encrypted images in the input folder and saves them to the output folder."""
    os.makedirs(output_folder, exist_ok=True)  # Ensure decrypted folder exists

    for filename in os.listdir(input_folder):
        if filename.endswith(".enc"):
            input_path = os.path.join(input_folder, filename)
            output_path = os.path.join(output_folder, filename.replace(".enc", ""))
            decrypt_image(input_path, output_path)

if __name__ == "__main__":
    decrypt_all_images("encrypted", "decrypted")

