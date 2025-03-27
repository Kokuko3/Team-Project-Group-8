from flask import Flask, render_template, send_from_directory
import os

app = Flask(__name__, template_folder=".")

# Images are in the same folder as the app
IMAGE_FOLDER = os.path.abspath(os.path.dirname(__file__))

@app.route('/')
def gallery():
    image_files = [f for f in os.listdir(IMAGE_FOLDER) if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif'))]
    return render_template("gallery.html", images=image_files)

@app.route('/<filename>')
def serve_image(filename):
    return send_from_directory(IMAGE_FOLDER, filename)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)
