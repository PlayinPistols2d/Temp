<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>Neon Air Hockey</title>
<style>
  body {
    margin: 0;
    background: #000;
    font-family: sans-serif;
    color: #0ff;
    user-select: none;
    overflow: hidden;
  }
  canvas {
    display: block;
    margin: 0 auto;
    background: #111;
  }
</style>
</head>
<body>
<canvas id="gameCanvas"></canvas>
<script>
(function() {
    const canvas = document.getElementById('gameCanvas');
    const ctx = canvas.getContext('2d');

    // Set canvas to full window size
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    // Game states
    const STATE_MENU = 0;
    const STATE_GAME = 1;
    const STATE_END = 2;
    let gameState = STATE_MENU;

    // Game parameters
    const PADDLE_WIDTH = 20;
    const PADDLE_HEIGHT = 100;
    const PADDLE_SPEED = 6;
    const PUCK_RADIUS = 12;
    const PUCK_SPEED = 7;
    const GOAL_LIMIT = 5;

    // Positions
    let leftPaddleY, rightPaddleY;
    let puckX, puckY;
    let puckVelX, puckVelY;

    // Scores
    let leftScore = 0;
    let rightScore = 0;

    // Key states
    const keys = {};
    window.addEventListener('keydown', e => keys[e.code] = true);
    window.addEventListener('keyup', e => keys[e.code] = false);

    // Resize handling
    window.addEventListener('resize', () => {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
        if (gameState === STATE_GAME) resetPositions();
    });

    function resetPositions() {
        leftPaddleY = canvas.height/2 - PADDLE_HEIGHT/2;
        rightPaddleY = canvas.height/2 - PADDLE_HEIGHT/2;
        puckX = canvas.width/2;
        puckY = canvas.height/2;
        // Random direction at start
        puckVelX = Math.random() > 0.5 ? PUCK_SPEED : -PUCK_SPEED;
        puckVelY = (Math.random() - 0.5)*2*PUCK_SPEED;
    }

    function resetGame() {
        leftScore = 0;
        rightScore = 0;
        resetPositions();
    }

    function update() {
        if (gameState === STATE_GAME) {
            handleInput();
            movePuck();
            checkScore();
        }
        draw();
        requestAnimationFrame(update);
    }

    function handleInput() {
        // Player 1: W,S for up/down, A,D for left/right (though usually not needed in air hockey)
        if (keys['KeyW']) {
            leftPaddleY -= PADDLE_SPEED;
        }
        if (keys['KeyS']) {
            leftPaddleY += PADDLE_SPEED;
        }
        // Player 2: Up/Down arrow keys
        if (keys['ArrowUp']) {
            rightPaddleY -= PADDLE_SPEED;
        }
        if (keys['ArrowDown']) {
            rightPaddleY += PADDLE_SPEED;
        }

        // Prevent paddles from going out of the arena
        leftPaddleY = Math.max(0, Math.min(canvas.height - PADDLE_HEIGHT, leftPaddleY));
        rightPaddleY = Math.max(0, Math.min(canvas.height - PADDLE_HEIGHT, rightPaddleY));
    }

    function movePuck() {
        puckX += puckVelX;
        puckY += puckVelY;

        // Collision with top and bottom
        if (puckY - PUCK_RADIUS < 0) {
            puckY = PUCK_RADIUS;
            puckVelY = -puckVelY;
        } else if (puckY + PUCK_RADIUS > canvas.height) {
            puckY = canvas.height - PUCK_RADIUS;
            puckVelY = -puckVelY;
        }

        // Check collision with left paddle
        if (puckX - PUCK_RADIUS < PADDLE_WIDTH + 10) { 
            // Check if within paddle height
            if (puckY > leftPaddleY && puckY < leftPaddleY + PADDLE_HEIGHT) {
                puckX = PADDLE_WIDTH + 10 + PUCK_RADIUS;
                puckVelX = -puckVelX;
                // Add some "english" based on where it hit the paddle
                let hitPos = (puckY - (leftPaddleY + PADDLE_HEIGHT/2)) / (PADDLE_HEIGHT/2);
                puckVelY += hitPos * 2; 
            }
        }

        // Check collision with right paddle
        if (puckX + PUCK_RADIUS > canvas.width - (PADDLE_WIDTH + 10)) {
            // Check if within paddle height
            if (puckY > rightPaddleY && puckY < rightPaddleY + PADDLE_HEIGHT) {
                puckX = canvas.width - (PADDLE_WIDTH + 10) - PUCK_RADIUS;
                puckVelX = -puckVelX;
                let hitPos = (puckY - (rightPaddleY + PADDLE_HEIGHT/2)) / (PADDLE_HEIGHT/2);
                puckVelY += hitPos * 2;
            }
        }
    }

    function checkScore() {
        // Check if puck passed left side
        if (puckX + PUCK_RADIUS < 0) {
            rightScore++;
            resetPositions();
        }

        // Check if puck passed right side
        if (puckX - PUCK_RADIUS > canvas.width) {
            leftScore++;
            resetPositions();
        }

        // Check for win condition
        if (leftScore >= GOAL_LIMIT || rightScore >= GOAL_LIMIT) {
            gameState = STATE_END;
        }
    }

    function drawNeonCircle(x, y, radius, color) {
        ctx.beginPath();
        let glow = ctx.createRadialGradient(x, y, radius*0.2, x, y, radius);
        glow.addColorStop(0, color);
        glow.addColorStop(1, 'transparent');
        ctx.fillStyle = glow;
        ctx.arc(x, y, radius, 0, Math.PI * 2);
        ctx.fill();
    }

    function drawNeonRect(x, y, w, h, color) {
        let glow = ctx.createLinearGradient(x, y, x+w, y+h);
        glow.addColorStop(0, color);
        glow.addColorStop(0.5, 'transparent');
        glow.addColorStop(1, color);

        ctx.strokeStyle = color;
        ctx.shadowBlur = 20;
        ctx.shadowColor = color;
        ctx.lineWidth = 3;
        ctx.strokeRect(x, y, w, h);
        
        ctx.shadowBlur = 0; // reset
    }

    function drawNeonLine(x1, y1, x2, y2, color) {
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.strokeStyle = color;
        ctx.shadowBlur = 20;
        ctx.shadowColor = color;
        ctx.lineWidth = 3;
        ctx.stroke();
        ctx.shadowBlur = 0;
    }

    function drawArena() {
        // Draw a fancy neon arena: center line, maybe some arcs, and a glowing boundary
        const centerX = canvas.width/2;
        const centerY = canvas.height/2;
        
        // Outer boundary
        drawNeonRect(5,5,canvas.width-10,canvas.height-10, '#0ff');

        // Center line
        drawNeonLine(centerX, 5, centerX, canvas.height-5, '#0ff');

        // Center circle
        drawNeonCircle(centerX, centerY, 50, '#0ff');
    }

    function drawPaddles() {
        // Draw paddles as neon rectangles
        const paddleGlow = '#f0f';
        ctx.fillStyle = paddleGlow;
        ctx.shadowBlur = 20;
        ctx.shadowColor = paddleGlow;

        // Left paddle
        ctx.fillRect(10, leftPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT);
        // Right paddle
        ctx.fillRect(canvas.width - 10 - PADDLE_WIDTH, rightPaddleY, PADDLE_WIDTH, PADDLE_HEIGHT);

        ctx.shadowBlur = 0;
    }

    function drawPuck() {
        drawNeonCircle(puckX, puckY, PUCK_RADIUS, '#ff0');
    }

    function drawScores() {
        ctx.font = '40px Arial';
        ctx.fillStyle = '#0ff';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.shadowBlur = 20;
        ctx.shadowColor = '#0ff';
        ctx.fillText(leftScore, canvas.width/4, 50);
        ctx.fillText(rightScore, 3*canvas.width/4, 50);
        ctx.shadowBlur = 0;
    }

    function drawMenu() {
        ctx.fillStyle = '#0ff';
        ctx.font = '60px Arial';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.shadowBlur = 20;
        ctx.shadowColor = '#0ff';
        ctx.fillText('NEON AIR HOCKEY', canvas.width/2, canvas.height/2 - 50);

        ctx.font = '30px Arial';
        ctx.fillText('Press Enter to Start', canvas.width/2, canvas.height/2 + 50);
        ctx.fillText('Player 1: W,A,S,D | Player 2: Arrow Keys', canvas.width/2, canvas.height/2 + 100);
        ctx.shadowBlur = 0;
    }

    function drawEndScreen() {
        ctx.fillStyle = '#0ff';
        ctx.font = '60px Arial';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.shadowBlur = 20;
        ctx.shadowColor = '#0ff';

        let winner = leftScore > rightScore ? 'Left Player Wins!' : 'Right Player Wins!';
        ctx.fillText(winner, canvas.width/2, canvas.height/2 - 50);

        ctx.font = '30px Arial';
        ctx.fillText('Press Enter to Return to Main Menu', canvas.width/2, canvas.height/2 + 50);
        ctx.shadowBlur = 0;
    }

    function draw() {
        ctx.clearRect(0,0,canvas.width,canvas.height);

        if (gameState === STATE_MENU) {
            drawMenu();
        } else if (gameState === STATE_GAME) {
            drawArena();
            drawPaddles();
            drawPuck();
            drawScores();
        } else if (gameState === STATE_END) {
            drawArena();
            drawEndScreen();
        }
    }

    // Handle state transitions
    window.addEventListener('keydown', e => {
        if (e.code === 'Enter') {
            if (gameState === STATE_MENU) {
                resetGame();
                gameState = STATE_GAME;
            } else if (gameState === STATE_END) {
                gameState = STATE_MENU;
            }
        }
    });

    // Initialize
    gameState = STATE_MENU;
    update();
})();
</script>
</body>
</html>