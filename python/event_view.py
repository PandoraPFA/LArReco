import csv
import numpy as np

from PIL import Image

class Event:
    def __init__(self, id, true_vertex, hits):
        self.id = id
        self.true_vertex = true_vertex
        self.hits = hits

def fill_pixel(data, width, height, x, w, rgb, window=1):
    # Fill a nxn block around the central value
    pw = (height - 1) - int(np.floor(w))
    px = int(np.floor(x))
    for row in range(pw - window, pw + (window + 1)):
        for col in range(px - window, px + (window + 1)):
            if (0 <= col < width) and (0 <= row < height):
                data[row, col] = rgb

def make_event_image(event, int_type):
    width, height = 1080, 1920
    data = np.zeros((height, width, 3), dtype=np.uint8)
    # Get x and w ranges
    x_min, w_min, _ = np.amin(event.hits, axis=0)
    x_max, w_max, _ = np.amax(event.hits, axis=0)
    # Allow for vertex outside of hits region
    x_min = np.minimum(event.true_vertex[0], x_min)
    x_max = np.maximum(event.true_vertex[0], x_max)
    w_min = np.minimum(event.true_vertex[2], w_min)
    w_max = np.maximum(event.true_vertex[2], w_max)
    # Map to pixel values
    pixel_x = (width - 1) * (event.hits[:,0] - x_min) / (x_max - x_min)
    pixel_w = (height - 1) * (event.hits[:,1] - w_min) / (w_max - w_min)
    pixels = np.array(list(zip(pixel_x, pixel_w)))
    # Populate the data array
    for pixel in pixels:
        fill_pixel(data, width, height, pixel[0], pixel[1], [0,0,255])
    # Map true vertex to pixel values
    pixel_vx = (width - 1) * (event.true_vertex[0] - x_min) / (x_max - x_min)
    pixel_vw = (height - 1) * (event.true_vertex[2] - w_min) / (w_max - w_min)
    # Populate the data array
    fill_pixel(data, width, height, pixel_vx, pixel_vw, [255,0,0], 3)
    # Convert to an image
    img = Image.fromarray(data, 'RGB')
    img.save("{}_event_{}.png".format(int_type, event.id))

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='Process some events')
    parser.add_argument("--input-file", "-i", dest="input_file", type=str,
        help="The input events file to process", required=True)
    args = parser.parse_args()

    with open(args.input_file) as csv_file:
        import os
        _, int_type = os.path.split(args.input_file)
        int_type, _ = os.path.splitext(int_type)
        # Read file contents into a dictionary
        csv_reader = csv.DictReader(csv_file, delimiter=',')
        # Process the file line-by-line
        events = []
        count = 0
        for row in csv_reader:
            # Store the true vertex as a tuple
            vertex = (float(row["true_vertex_x"]), float(row["true_vertex_y"]), float(row["true_vertex_z"]))
            # All (x, wire, energy) triplets are stored as a single 'column' in CSV
            # Take this column and convert it to a numpy array
            raw_hits = np.fromstring(row["hits(x wire energy)"], dtype=float, sep=" ")
            # Extract x, wire and energy values from raw array and zip them into array of triplets
            hits = np.array(list(zip(raw_hits[0::3], raw_hits[1::3], raw_hits[2::3])))
            # Store the event
            events.append(Event(count, vertex, hits))
            count += 1
        
        for event in events:
            make_event_image(event, int_type)
