from flask import Flask, render_template, send_from_directory, url_for
import os

app = Flask(__name__, template_folder=".")

# ✅ Change this to the actual absolute path to your image folder
IMAGE_FOLDER = os.path.abspath("/home/temp/Flask-Testing")

@app.route('/')
def gallery():
    image_files = [f for f in os.listdir(IMAGE_FOLDER) if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif'))]
    return render_template("gallery2.html", images=image_files)

@app.route('/images/<filename>')
def serve_image(filename):
    return send_from_directory(IMAGE_FOLDER, filename)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)

