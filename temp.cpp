<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>Neon Retrowave Car Game</title>
<style>
  html, body {
    margin: 0; 
    padding: 0; 
    overflow: hidden; 
    background: #000;
    font-family: 'Courier New', Courier, monospace;
    color: #fff;
  }

  #gameCanvas {
    display: block;
    margin: 0 auto;
    background: #000;
    border: 2px solid #ff00ff;
  }

  .overlay {
    position: absolute;
    top: 10px;
    left: 10px;
    font-size: 16px;
    color: #ff00ff;
    text-shadow: 0 0 5px #ff00ff;
  }
</style>
</head>
<body>
<canvas id="gameCanvas" width="800" height="600"></canvas>
<div class="overlay" id="hud">
  Score: 0<br>
  Lives: 3
</div>
<script>
(function() {
  // Canvas and Context Setup
  const canvas = document.getElementById('gameCanvas');
  const ctx = canvas.getContext('2d');
  const W = canvas.width;
  const H = canvas.height;

  // Game Constants
  const ROAD_WIDTH = 300;
  const LANE_COUNT = 3;
  const CAR_WIDTH = 40;
  const CAR_HEIGHT = 70;
  const CAR_SPEED = 5;
  const OBSTACLE_WIDTH = 40;
  const OBSTACLE_HEIGHT = 40;
  const COIN_SIZE = 20;

  // Game State
  let score = 0;
  let lives = 3;
  let gameOver = false;

  // Player car initial position (center lane)
  let playerX = W / 2;
  let playerY = H - CAR_HEIGHT * 2;

  // Input Controls
  let moveLeft = false;
  let moveRight = false;

  // Road scroll offset
  let roadOffset = 0;

  // Arrays for obstacles and coins
  let obstacles = [];
  let coins = [];

  // Neon Color Palette
  const neonPurple = '#ff00ff';
  const neonCyan = '#00ffff';
  const neonPink = '#ff0090';
  const neonYellow = '#ffff00';

  // Helper: Generate random lane X coordinate
  function laneX(laneIndex) {
    // laneIndex in [0, LANE_COUNT-1]
    // center the road and distribute lanes
    const laneWidth = ROAD_WIDTH / LANE_COUNT;
    const startX = (W / 2) - (ROAD_WIDTH / 2);
    return startX + laneWidth * laneIndex + laneWidth / 2;
  }

  // Generate obstacles
  function spawnObstacle() {
    const laneIndex = Math.floor(Math.random() * LANE_COUNT);
    obstacles.push({
      x: laneX(laneIndex) - OBSTACLE_WIDTH / 2,
      y: -OBSTACLE_HEIGHT,
      speed: 6 + Math.random() * 3,
    });
  }

  // Generate coins
  function spawnCoin() {
    const laneIndex = Math.floor(Math.random() * LANE_COUNT);
    coins.push({
      x: laneX(laneIndex) - COIN_SIZE / 2,
      y: -COIN_SIZE,
      speed: 6 + Math.random() * 3,
    });
  }

  // Periodic spawns
  let obstacleTimer = 0;
  let coinTimer = 0;
  const obstacleSpawnInterval = 100;
  const coinSpawnInterval = 150;

  // Input events
  document.addEventListener('keydown', e => {
    if (e.key === 'ArrowLeft' || e.key === 'a') moveLeft = true;
    if (e.key === 'ArrowRight' || e.key === 'd') moveRight = true;
  });
  document.addEventListener('keyup', e => {
    if (e.key === 'ArrowLeft' || e.key === 'a') moveLeft = false;
    if (e.key === 'ArrowRight' || e.key === 'd') moveRight = false;
  });

  // Collision check helper
  function rectCollision(ax, ay, aw, ah, bx, by, bw, bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
  }

  // Draw Retro Sunset Gradient Background
  function drawBackground() {
    // Sky gradient (sunset feel)
    const grad = ctx.createLinearGradient(0, 0, 0, H);
    grad.addColorStop(0, '#270031'); // dark top
    grad.addColorStop(0.3, '#ff00ff'); 
    grad.addColorStop(0.6, '#ff0090');
    grad.addColorStop(1, '#001144'); // bluish bottom
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, W, H);

    // Draw a neon sun on the horizon
    const sunRadius = 80;
    const sunX = W / 2;
    const sunY = H * 0.3;
    const sunGrad = ctx.createRadialGradient(sunX, sunY, sunRadius*0.1, sunX, sunY, sunRadius);
    sunGrad.addColorStop(0, neonYellow);
    sunGrad.addColorStop(1, 'rgba(255,0,255,0)');
    ctx.fillStyle = sunGrad;
    ctx.beginPath();
    ctx.arc(sunX, sunY, sunRadius, 0, Math.PI * 2);
    ctx.fill();
  }

  // Draw Road
  function drawRoad() {
    // Road trapezoid perspective (simple)
    const roadTopY = H * 0.4;
    const roadBottomY = H;
    const roadHalf = ROAD_WIDTH / 2;

    // Road color
    ctx.fillStyle = '#000';
    ctx.beginPath();
    ctx.moveTo((W/2)-roadHalf*0.3, roadTopY);
    ctx.lineTo((W/2)-roadHalf, roadBottomY);
    ctx.lineTo((W/2)+roadHalf, roadBottomY);
    ctx.lineTo((W/2)+roadHalf*0.3, roadTopY);
    ctx.closePath();
    ctx.fill();

    // Road lines (simulate perspective with scaling)
    ctx.strokeStyle = neonCyan;
    ctx.lineWidth = 2;
    ctx.setLineDash([20, 10]); // dashed line
    ctx.lineDashOffset = roadOffset;
    ctx.beginPath();
    // Middle lane line
    ctx.moveTo(W/2, roadTopY);
    ctx.lineTo(W/2, roadBottomY);
    ctx.stroke();

    // Lane lines
    ctx.setLineDash([10, 10]);
    const leftLaneXTop = (W/2) - roadHalf*0.1;
    const leftLaneXBottom = (W/2)-roadHalf*0.66;
    const rightLaneXTop = (W/2) + roadHalf*0.1;
    const rightLaneXBottom = (W/2)+roadHalf*0.66;

    // Left lane line
    ctx.beginPath();
    ctx.moveTo(leftLaneXTop, roadTopY);
    ctx.lineTo(leftLaneXBottom, roadBottomY);
    ctx.stroke();

    // Right lane line
    ctx.beginPath();
    ctx.moveTo(rightLaneXTop, roadTopY);
    ctx.lineTo(rightLaneXBottom, roadBottomY);
    ctx.stroke();

    ctx.setLineDash([]);
  }

  // Draw Car (player)
  function drawCar() {
    // Player car as a neon trapezoid with glowing edges
    ctx.fillStyle = neonPink;
    ctx.shadowColor = neonPink;
    ctx.shadowBlur = 15;
    ctx.beginPath();
    ctx.rect(playerX - CAR_WIDTH/2, playerY, CAR_WIDTH, CAR_HEIGHT);
    ctx.fill();
    ctx.shadowBlur = 0; // reset shadow
  }

  // Draw obstacles
  function drawObstacles() {
    ctx.fillStyle = neonPurple;
    ctx.shadowColor = neonPurple;
    ctx.shadowBlur = 10;
    obstacles.forEach(o => {
      ctx.beginPath();
      ctx.rect(o.x, o.y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
      ctx.fill();
    });
    ctx.shadowBlur = 0;
  }

  // Draw coins
  function drawCoins() {
    ctx.fillStyle = neonYellow;
    ctx.shadowColor = neonYellow;
    ctx.shadowBlur = 15;
    coins.forEach(c => {
      ctx.beginPath();
      ctx.arc(c.x + COIN_SIZE/2, c.y + COIN_SIZE/2, COIN_SIZE/2, 0, Math.PI*2);
      ctx.fill();
    });
    ctx.shadowBlur = 0;
  }

  // Update game logic
  function update() {
    if (gameOver) return;

    // Move player
    if (moveLeft && playerX > (W/2 - ROAD_WIDTH/2)+CAR_WIDTH) {
      playerX -= CAR_SPEED;
    }
    if (moveRight && playerX < (W/2 + ROAD_WIDTH/2)-CAR_WIDTH) {
      playerX += CAR_SPEED;
    }

    // Move obstacles and coins down
    obstacles.forEach(o => o.y += o.speed);
    coins.forEach(c => c.y += c.speed);

    // Road offset for dashed lines
    roadOffset += 5;
    if (roadOffset > 30) roadOffset = 0;

    // Remove obstacles and coins out of screen
    obstacles = obstacles.filter(o => o.y < H + OBSTACLE_HEIGHT);
    coins = coins.filter(c => c.y < H + COIN_SIZE);

    // Check collisions
    const px = playerX - CAR_WIDTH/2;
    const py = playerY;
    obstacles.forEach((o, i) => {
      if (rectCollision(px, py, CAR_WIDTH, CAR_HEIGHT, o.x, o.y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT)) {
        // Lose a life
        obstacles.splice(i,1);
        lives--;
        if (lives <= 0) {
          lives = 0;
          gameOver = true;
        }
      }
    });

    coins.forEach((c, i) => {
      if (rectCollision(px, py, CAR_WIDTH, CAR_HEIGHT, c.x, c.y, COIN_SIZE, COIN_SIZE)) {
        // Collect coin
        coins.splice(i,1);
        score += 10;
      }
    });

    // Spawning new obstacles and coins
    obstacleTimer++;
    if (obstacleTimer > obstacleSpawnInterval) {
      spawnObstacle();
      obstacleTimer = 0;
    }

    coinTimer++;
    if (coinTimer > coinSpawnInterval) {
      spawnCoin();
      coinTimer = 0;
    }

    // Update HUD
    const hud = document.getElementById('hud');
    hud.innerHTML = `Score: ${score}<br>Lives: ${lives}${gameOver ? '<br><span style="color:#ff0000;">GAME OVER</span>' : ''}`;
  }

  // Render
  function render() {
    // Clear
    ctx.clearRect(0, 0, W, H);

    // Draw everything
    drawBackground();
    drawRoad();
    drawCar();
    drawObstacles();
    drawCoins();
  }

  // Main loop
  function gameLoop() {
    update();
    render();
    requestAnimationFrame(gameLoop);
  }

  // Start
  gameLoop();
})();
</script>
</body>
</html>