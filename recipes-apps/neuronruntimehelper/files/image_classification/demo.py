from NeuronRuntimeHelper import NeuronContext
import argparse
import numpy as np
import cv2

def main(model_path, image_path, labels_path):
    try:
        context = NeuronContext(model_path)

        # Initialize model
        if not context.Initialize():
            raise RuntimeError("Failed to initialize NeuronRuntime")

        # Get model input/output Dimensions NHWC
        input_dims = context.GetInputDimensions()
        output_dims = context.GetOutputDimensions()
        print(f"model input dimensions: {input_dims}")
        print(f"model output dimensions: {output_dims}")
        
        input_data_types = context.GetInputDataType()
        output_data_types = context.GetOutputDataType()
        print(f"input data types: {input_data_types}")
        print(f"output data types: {output_data_types}")

        # Load input image
        _, height, width, channels = input_dims[0]
        original_image = cv2.imread(image_path)
        rgb_image = cv2.cvtColor(original_image, cv2.COLOR_BGR2RGB)
        rgb_image = cv2.resize(rgb_image, (width, height))

        # Preprocess input image
        #input = np.array(rgb_image, dtype=np.float32)
        #input /= 255.0
        if input_data_types[0] == 'FLOAT32':
            input = np.array(rgb_image, dtype=np.float32) / 255.0
        elif input_data_types[0] == 'UINT8':
            input = np.array(rgb_image, dtype=np.uint8)
        elif input_data_types[0] == 'INT8':
            input = np.array(rgb_image, dtype=np.int8)
        else:
            raise ValueError(f"Unsupported input data type: {input_data_types[0]}")

        # Set input buffer for inference
        context.SetInputBuffer(input, 0)

        # Execute model
        if not context.Execute():
            raise RuntimeError("Model execution failed")

        # Postprocess output
        output_data = context.GetOutputBuffer(0)
        print(f"output shape: {output_data.shape}")
        output_data = output_data.flatten()
        
        with open(labels_path, 'r') as f:
            labels = [line.strip() for line in f.readlines()]

        top_5_indices = np.argsort(output_data)[-5:][::-1]
        top_5_confidences = output_data[top_5_indices]
        top_5_labels = [labels[i] for i in top_5_indices]

        colors = [(0, 255, 0), (0, 0, 255), (255, 0, 0), (0, 255, 255), (255, 0, 255)]
        for i, (label, confidence) in enumerate(zip(top_5_labels, top_5_confidences)):
            text = f"{label}: {confidence:.4f}"
            cv2.putText(original_image, text, (10, 30 + 30 * i), cv2.FONT_HERSHEY_SIMPLEX, 0.8, colors[i % len(colors)], 2)

        cv2.imshow("Image Classification", original_image)
        cv2.waitKey(8000)
        cv2.destroyAllWindows()
    except KeyboardInterrupt:
        print("User interrupted the program. Exiting...")
        cv2.destroyAllWindows()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Image classification example using NeuronRuntimeHelper')
    parser.add_argument('--model', type=str, default='mobilenetv2_u8_224.dla',
                        help='Path to the DLA file')
    parser.add_argument('--image', type=str, default='grace_hopper.jpg',
                        help='Path to the input image')
    parser.add_argument('--labels', type=str, default='labels.txt',
                        help='Path to the labels file')
    args = parser.parse_args()

    main(args.model, args.image, args.labels)