from NeuronRuntimeHelper import NeuronContext
import argparse
import numpy as np
import cv2

class YOLOv8(NeuronContext):
    def __init__(self, dla_path: str = "None", labels_path: str = "None", confidence_thres=0.5, iou_thres=0.5):
        super().__init__(dla_path)
        self.confidence_thres = confidence_thres
        self.iou_thres = iou_thres

        self.labels = None
        with open(labels_path, 'r') as f:
            self.labels = [line.strip() for line in f.readlines()]

    def draw_boxes(self, image, box, score, class_id):
        x1, y1, w, h = [int(v) for v in box]
        color = [0, 255, 0]  # green
        cv2.rectangle(image, (x1, y1), (x1 + w, y1 + h), color, 2)
        
        class_name = self.labels[class_id] if self.labels else str(class_id)
        label = f"{class_name}: {score:.2f}"
        (label_width, label_height), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
        label_x = x1
        label_y = y1 - 10 if y1 - 10 > label_height else y1 + 10
        cv2.rectangle(
            image,
            (int(label_x), int(label_y - label_height)),
            (int(label_x + label_width), int(label_y + label_height)),
            color,
            cv2.FILLED,
        )
        # Add the label text
        cv2.putText(
            image,
            label,
            (int(label_x), int(label_y)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,  # font scale
            (0, 0, 0),  # text color
            1,  # line thickness
            cv2.LINE_AA,
        )

    def inference(self, input_data):
        self.SetInputBuffer(input_data, 0)
        return self.Execute()

    def preprocess(self, image):
        # Get model input/output Dimensions NHWC
        input_dims = self.GetInputDimensions()
        output_dims = self.GetOutputDimensions()
        print(f"model input dimensions: {input_dims}")
        print(f"model output dimensions: {output_dims}")
        
        input_data_types = self.GetInputDataType()
        output_data_types = self.GetOutputDataType()
        print(f"input data types: {input_data_types}")
        print(f"output data types: {output_data_types}")

        # Load input image
        _, self.model_height, self.model_width, self.channels = input_dims[0]
        rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        rgb_image = cv2.resize(rgb_image, (self.model_width, self.model_height))

        # Preprocess input image
        if input_data_types[0] == 'FLOAT32':
            input_data = np.array(rgb_image, dtype=np.float32) / 255.0
        elif input_data_types[0] == 'UINT8':
            input_data = np.array(rgb_image, dtype=np.uint8)
        else:
            raise ValueError(f"Unsupported input data type: {input_data_types[0]}")

        return input_data

    def postprocess(self, image):
        original_height, original_width = image.shape[:2]
        scale_x = original_width / self.model_width
        scale_y = original_height / self.model_height
        output_buf = self.GetOutputBuffer(0)
        boxes = []
        scores = []
        class_ids = []

        outputs = np.array([cv2.transpose(output_buf[0])])
        rows = outputs.shape[1]
        for i in range(rows):
            classes_scores = outputs[0][i][4:]
            (minScore, maxScore, minClassLoc, (x, maxClassIndex)) = cv2.minMaxLoc(classes_scores)
            if maxScore >= 0.25:
                box = [
                    (outputs[0][i][0] - (0.5 * outputs[0][i][2])) * self.model_width * scale_x,
                    (outputs[0][i][1] - (0.5 * outputs[0][i][3])) * self.model_height * scale_y,
                    outputs[0][i][2] * self.model_width * scale_x,
                    outputs[0][i][3] * self.model_height * scale_y,
                ]
                boxes.append(box)
                scores.append(maxScore)
                class_ids.append(maxClassIndex)

        indices = cv2.dnn.NMSBoxes(
            boxes, scores, self.confidence_thres, self.iou_thres
        )    

        for i in indices:
            box = boxes[i]
            score = scores[i]
            class_id = class_ids[i]

            if score > 0.4:
                self.draw_boxes(image, box, score, class_id)
        cv2.imshow("Object Detection", image)

def main(dla_path, image_path, labels_path):
    model = YOLOv8(dla_path=dla_path, labels_path=labels_path)
    ret = model.Initialize()
    if ret != True:
        print("Failed to initialize model")
        return

    original_image = cv2.imread(image_path)
    input_data = model.preprocess(original_image)

    if model.inference(input_data) != True:
        print("Failed to inference")
        return

    model.postprocess(original_image)

    cv2.waitKey(8000)
    cv2.destroyAllWindows()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='YOLOv8 example using NeuronRuntimeHelper')
    parser.add_argument('--dla-path', type=str, default='yolov8n_u8_640.dla',
                        help='Path to the YOLOv8 DLA file')
    parser.add_argument('--image-path', type=str, default='bus.jpg',
                        help='Path to the input image')
    parser.add_argument('--labels', type=str, default='labels.txt',
                        help='Path to the labels file')
    args = parser.parse_args()

    main(args.dla_path, args.image_path, args.labels)
