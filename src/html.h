#include <pgmspace.h>

const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EPD Image Upload - Method A</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background: #f0f0f0;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
        }
        .upload-area {
            border: 3px dashed #ccc;
            border-radius: 10px;
            padding: 40px;
            text-align: center;
            cursor: pointer;
            margin: 20px 0;
        }
        .upload-area:hover {
            border-color: #666;
            background: #f9f9f9;
        }
        #preview {
            max-width: 100%;
            margin: 20px 0;
            display: none;
            border: 1px solid #ddd;
        }
        .controls {
            margin: 20px 0;
        }
        .control-group {
            margin: 15px 0;
        }
        label {
            display: inline-block;
            width: 150px;
            font-weight: bold;
        }
        input[type="range"] {
            width: 300px;
        }
        button {
            background: #4CAF50;
            color: white;
            border: none;
            padding: 15px 30px;
            font-size: 16px;
            border-radius: 5px;
            cursor: pointer;
            width: 100%;
            margin-top: 20px;
        }
        button:hover {
            background: #45a049;
        }
        button:disabled {
            background: #ccc;
            cursor: not-allowed;
        }
        .status {
            padding: 15px;
            margin: 15px 0;
            border-radius: 5px;
            display: none;
        }
        .status.info {
            background: #e3f2fd;
            border: 1px solid #2196F3;
            color: #1976D2;
        }
        .status.success {
            background: #e8f5e9;
            border: 1px solid #4CAF50;
            color: #2e7d32;
        }
        .status.error {
            background: #ffebee;
            border: 1px solid #f44336;
            color: #c62828;
        }
        .progress {
            margin: 15px 0;
            display: none;
        }
        .progress-bar {
            width: 100%;
            height: 30px;
            background: #f0f0f0;
            border-radius: 15px;
            overflow: hidden;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #4CAF50, #45a049);
            width: 0%;
            transition: width 0.3s;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
        }
        .info-box {
            background: #fff3cd;
            border: 1px solid #ffc107;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🖼️ E-Paper Display Uploader</h1>
        
        <div class="upload-area" onclick="document.getElementById('fileInput').click()">
            📁 Click to choose picture<br>
            <small>or drag and drop an image</small>
        </div>
        <input type="file" id="fileInput" accept="image/*" style="display:none">
        
        <canvas id="preview" width="1200" height="1600"></canvas>
        
        <div class="controls">
            <div class="control-group">
                <label>Contrast:</label>
                <input type="range" id="contrast" min="50" max="200" value="120">
                <span id="contrastValue">120%</span>
            </div>
            <div class="control-group">
                <label>Saturation:</label>
                <input type="range" id="saturation" min="50" max="200" value="130">
                <span id="saturationValue">130%</span>
            </div>
            <div class="control-group">
                <button type="button" onclick="rotatePreview()">
                🔄 Rotate 90°
                </button>
            </div>
        </div>
        
        <button id="uploadBtn" disabled onclick="processAndUpload()">
            Process and Upload
        </button>
        
        <div class="progress" id="progress">
            <div class="progress-bar">
                <div class="progress-fill" id="progressFill">0%</div>
            </div>
        </div>
        
        <div class="status" id="status"></div>
    </div>

    <script>
        let originalImage = null;
        let rotation = 0;
        const canvas = document.getElementById('preview');
        const ctx = canvas.getContext('2d');
        
        // 6 Color palette for EPD
        const EPD_COLORS = {
            BLACK:  {r: 0,   g: 0,   b: 0,   value: 0x00},
            WHITE:  {r: 255, g: 255, b: 255, value: 0x11},
            YELLOW: {r: 255, g: 255, b: 0,   value: 0x22},
            RED:    {r: 255, g: 0,   b: 0,   value: 0x33},
            BLUE:   {r: 0,   g: 0,   b: 255, value: 0x55},
            GREEN:  {r: 0,   g: 255, b: 0,   value: 0x66}
        };
        
        // Choose image
        document.getElementById('fileInput').addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (file) {
                loadImage(file);
            }
        });
        
        // Drag and drop
        const uploadArea = document.querySelector('.upload-area');
        uploadArea.addEventListener('dragover', (e) => {
            e.preventDefault();
            uploadArea.style.borderColor = '#4CAF50';
        });
        uploadArea.addEventListener('dragleave', () => {
            uploadArea.style.borderColor = '#ccc';
        });
        uploadArea.addEventListener('drop', (e) => {
            e.preventDefault();
            uploadArea.style.borderColor = '#ccc';
            const file = e.dataTransfer.files[0];
            if (file && file.type.startsWith('image/')) {
                loadImage(file);
            }
        });
        
        // Controls
        document.getElementById('contrast').addEventListener('input', function(e) {
            document.getElementById('contrastValue').textContent = e.target.value + '%';
            if (originalImage) processImage();
        });
        document.getElementById('saturation').addEventListener('input', function(e) {
            document.getElementById('saturationValue').textContent = e.target.value + '%';
            if (originalImage) processImage();
        });
        
        function loadImage(file) {
            showStatus('Loading image...', 'info');
            const reader = new FileReader();
            reader.onload = function(e) {
                const img = new Image();
                img.onload = function() {
                    originalImage = img;
                    processImage();
                    document.getElementById('uploadBtn').disabled = false;
                    showStatus('Image loaded successfully. Please adjust the parameters.', 'success');
                };
                img.src = e.target.result;
            };
            reader.readAsDataURL(file);
        }
        
        function rotatePreview() {
            rotation = (rotation - 90) % 360;
            processImage();
        }
        
        function processImage() {
            if (!originalImage) return;
            
            showStatus('Processing image...', 'info');

            const CW = canvas.width;   //  1200
            const CH = canvas.height;  //  1600
            const rad = rotation * Math.PI / 180;

            // calculate the size of the bounding box after rotation
            const sin = Math.abs(Math.sin(rad));
            const cos = Math.abs(Math.cos(rad));
            const iw = originalImage.naturalWidth;
            const ih = originalImage.naturalHeight;
            
            const scaleX = CW / (iw * cos + ih * sin);
            const scaleY = CH / (iw * sin + ih * cos);
            const scale  = Math.max(scaleX, scaleY); // crop to fill
            ctx.save();
            ctx.translate(CW / 2, CH / 2);
            ctx.rotate(rad);
            ctx.drawImage(originalImage, -iw * scale / 2, -ih * scale / 2, iw * scale, ih * scale);
            ctx.restore();
            
            // adjust contrast and saturation
            const imageData = ctx.getImageData(0, 0, 1200, 1600);
            adjustImage(imageData);
            ctx.putImageData(imageData, 0, 0);
            
            canvas.style.display = 'block';
            showStatus('Preview completed', 'success');
        }
        
        function adjustImage(imageData) {
            const contrast = document.getElementById('contrast').value / 100;
            const saturation = document.getElementById('saturation').value / 100;
            const data = imageData.data;
            
            for (let i = 0; i < data.length; i += 4) {
                let r = data[i];
                let g = data[i + 1];
                let b = data[i + 2];
                
                // Contrast adjustment
                r = ((r / 255 - 0.5) * contrast + 0.5) * 255;
                g = ((g / 255 - 0.5) * contrast + 0.5) * 255;
                b = ((b / 255 - 0.5) * contrast + 0.5) * 255;
                
                // Saturation adjustment
                const gray = 0.299 * r + 0.587 * g + 0.114 * b;
                r = gray + (r - gray) * saturation;
                g = gray + (g - gray) * saturation;
                b = gray + (b - gray) * saturation;
                
                // Clamp values
                data[i] = Math.max(0, Math.min(255, r));
                data[i + 1] = Math.max(0, Math.min(255, g));
                data[i + 2] = Math.max(0, Math.min(255, b));
            }
        }
        
        function reorderForEPD(data) {

		    const width = 1200;
		    const height = 1600;

		    const bytesPerLine = width / 2;   // 600
    		const halfLine = bytesPerLine / 2; // 300

    		const output = new Uint8Array(data.length);

    		let leftPtr = 0;
 		   	let rightPtr = 480000;

    		for (let y = 0; y < height; y++) {

        	let rowStart = y * bytesPerLine;

        	// Left 600px
        	output.set(
            	data.slice(rowStart, rowStart + halfLine),
            	leftPtr
        	);
        	leftPtr += halfLine;

        	// Right 600px
        	output.set(
            data.slice(rowStart + halfLine, rowStart + bytesPerLine),
            	rightPtr
        	);
        	rightPtr += halfLine;
    		}

    		return output;
		}
        
        async function processAndUpload() {
            if (!originalImage) return;
            
            document.getElementById('uploadBtn').disabled = true;
            showProgress(0);
            showStatus('Dithering processing...', 'info');
            
            // Dithering and reordering
            const imageData = ctx.getImageData(0, 0, 1200, 1600);
            const ditheredData = await ditherImage(imageData);
            const resultData = reorderForEPD(ditheredData);
            
            showProgress(50);
            showStatus('Transferring to ESP32... (960KB)', 'info');
            
            // Transfer to ESP32 using Fetch API with FormData
            try {
        		const formData = new FormData();
        		// Wrap the ArrayBuffer in a Blob to send as multipart/form-data
        		formData.append('file', new Blob([resultData], {type: 'application/octet-stream'}), 'image.bin');

	        	const response = await fetch('/upload', {
	            	method: 'POST',
    	        	body: formData
    	    	});

        		showProgress(100);

        		if (response.ok) {
            		showStatus('✓ Transfer completed! Displaying on EPD...', 'success');
        		} else {
            		showStatus('✗ Transfer failed: ' + response.statusText, 'error');
        		}
    		} catch (error) {
        		showStatus('✗ Error: ' + error.message, 'error');
    		} finally {
        		document.getElementById('uploadBtn').disabled = false;
        		setTimeout(() => hideProgress(), 2000);
    		}
        }
        
        async function ditherImage(imageData) {
            const width = 1200;
            const height = 1600;
            const data = new Uint8ClampedArray(imageData.data);
            
            // Atkinson dithering
            for (let y = 0; y < height; y++) {
                for (let x = 0; x < width; x++) {
                    const idx = (y * width + x) * 4;
                    
                    const oldR = data[idx];
                    const oldG = data[idx + 1];
                    const oldB = data[idx + 2];
                    
                    // Find closest color in the 6-color palette
                    const newColor = findClosestColor(oldR, oldG, oldB);
                    
                    data[idx] = newColor.r;
                    data[idx + 1] = newColor.g;
                    data[idx + 2] = newColor.b;
                    
                    // Distribute error
                    const errR = oldR - newColor.r;
                    const errG = oldG - newColor.g;
                    const errB = oldB - newColor.b;
                    
                    distributeError(data, width, height, x, y, errR, errG, errB);
                }
                
                // Progress update
                if (y % 100 === 0) {
                    showProgress(Math.floor((y / height) * 50));
                    await new Promise(resolve => setTimeout(resolve, 0));
                }
            }
            
            // Convert to 4bit/pixel format
            const outputSize = (width * height) / 2;
            const output = new Uint8Array(outputSize);
            
            for (let i = 0; i < width * height; i++) {
                const idx = i * 4;
                const color = findClosestColor(data[idx], data[idx + 1], data[idx + 2]);
                
                const byteIdx = Math.floor(i / 2);
                if (i % 2 === 0) {
                    output[byteIdx] = color.value << 4;
                } else {
                    output[byteIdx] |= color.value;
                }
            }
            
            return output;
        }
        
        function findClosestColor(r, g, b) {
            let minDist = Infinity;
            let closest = EPD_COLORS.WHITE;
            
            for (const color of Object.values(EPD_COLORS)) {
                const dist = Math.abs(r - color.r) + Math.abs(g - color.g) + Math.abs(b - color.b);
                if (dist < minDist) {
                    minDist = dist;
                    closest = color;
                }
            }
            
            return closest;
        }
        
        function distributeError(data, width, height, x, y, errR, errG, errB) {
            const distribute = (dx, dy, factor) => {
                const nx = x + dx;
                const ny = y + dy;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    const idx = (ny * width + nx) * 4;
                    data[idx] = Math.max(0, Math.min(255, data[idx] + errR * factor));
                    data[idx + 1] = Math.max(0, Math.min(255, data[idx + 1] + errG * factor));
                    data[idx + 2] = Math.max(0, Math.min(255, data[idx + 2] + errB * factor));
                }
            };
            
            distribute(1, 0, 1/8);
            distribute(2, 0, 1/8);
            distribute(-1, 1, 1/8);
            distribute(0, 1, 1/8);
            distribute(0, 2, 1/8);
            distribute(1, 1, 1/8);
        }
        
        function showStatus(message, type) {
            const status = document.getElementById('status');
            status.textContent = message;
            status.className = 'status ' + type;
            status.style.display = 'block';
        }
        
        function showProgress(percent) {
            document.getElementById('progress').style.display = 'block';
            document.getElementById('progressFill').style.width = percent + '%';
            document.getElementById('progressFill').textContent = percent + '%';
        }
        
        function hideProgress() {
            document.getElementById('progress').style.display = 'none';
        }
    </script>
</body>
</html>
)rawliteral";