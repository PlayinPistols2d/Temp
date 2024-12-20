<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>Neon Retrowave Car Game</title>
<link rel="stylesheet" href="style.css"/>
</head>
<body>
<canvas id="gameCanvas" width="800" height="600"></canvas>
<div class="overlay" id="hud">
  Score: 0<br>
  Lives: 3
</div>
<script src="script.js"></script>
</body>
</html>





html, body {
  margin: 0; 
  padding: 0; 
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
  const OBSTACLE_WIDTH = 40;
  const OBSTACLE_HEIGHT = 40;
  const COIN_SIZE = 20;

  // Perspective/Camera
  const horizonY = H * 0.4;    // horizon line
  const fov = 300;             // field of view
  const playerZ = 0;           // player at z=0
  const laneWidth = ROAD_WIDTH / LANE_COUNT;
  const laneOffsets = [ -laneWidth, 0, laneWidth ]; // Lanes relative to center

  // Game State
  let score = 0;
  let lives = 3;
  let gameOver = false;

  // Player lane (0=left,1=middle,2=right)
  let playerLane = 1;

  // Forward speed (movement of objects towards player)
  let forwardSpeed = 10;

  // Objects array: { lane: number, z: number, type:'obstacle'|'coin' }
  let objects = [];

  // Neon Color Palette
  const neonPurple = '#ff00ff';
  const neonCyan   = '#00ffff';
  const neonPink   = '#ff0090';
  const neonYellow = '#ffff00';

  // Spawn intervals and initial distances
  let obstacleTimer = 0;
  let coinTimer = 0;
  const obstacleSpawnInterval = 100;  
  const coinSpawnInterval = 150;     
  const initialSpawnDistance = 2000; 

  // Key Press: Change lane
  document.addEventListener('keydown', e => {
    if ((e.key === 'ArrowLeft' || e.key === 'a') && playerLane > 0) {
      playerLane -= 1;
    }
    if ((e.key === 'ArrowRight' || e.key === 'd') && playerLane < LANE_COUNT - 1) {
      playerLane += 1;
    }
  });

  // Lane X helper
  function laneX(laneIndex) {
    return laneOffsets[laneIndex];
  }

  // Project point from (x,z) to screen (x,y)
  function projectPoint(x, z) {
    const scale = fov / (fov + z);
    const screenX = (W/2) + x * scale;
    const screenY = horizonY + (H - horizonY)*scale;
    return {x: screenX, y: screenY, scale: scale};
  }

  function spawnObstacle() {
    const laneIndex = Math.floor(Math.random() * LANE_COUNT);
    objects.push({ lane: laneIndex, z: initialSpawnDistance, type:'obstacle' });
  }

  function spawnCoin() {
    const laneIndex = Math.floor(Math.random() * LANE_COUNT);
    objects.push({ lane: laneIndex, z: initialSpawnDistance, type:'coin' });
  }

  // Check collisions
  function checkCollisions() {
    const toRemove = [];
    // Player rect
    // Player is drawn at bottom, based on playerLane
    // Let's figure out player's screen position. Player is effectively at z=0, x= laneX(playerLane)
    const playerXOffset = laneX(playerLane);
    const pProj = projectPoint(playerXOffset, playerZ); 
    // At z=0, scale = fov/(fov+0)=1, so pProj scale ~1. But we rely on the fixed bottom position:
    const playerScreenX = pProj.x; 
    const playerScreenY = H - CAR_HEIGHT - 20; 
    const playerRect = {
      x1: playerScreenX - CAR_WIDTH/2,
      y1: playerScreenY,
      x2: playerScreenX + CAR_WIDTH/2,
      y2: playerScreenY + CAR_HEIGHT
    };

    objects.forEach((obj, i) => {
      const x = laneX(obj.lane);
      const {x:ox, y:oy, scale} = projectPoint(x, obj.z);
      let ow, oh;
      if (obj.type === 'obstacle') {
        ow = OBSTACLE_WIDTH * scale;
        oh = OBSTACLE_HEIGHT * scale;
      } else {
        ow = COIN_SIZE * scale;
        oh = COIN_SIZE * scale;
      }

      const ox1 = ox - ow/2;
      const oy1 = oy - oh;
      const ox2 = ox + ow/2;
      const oy2 = oy;

      if (rectOverlap(playerRect.x1, playerRect.y1, playerRect.x2, playerRect.y2, ox1, oy1, ox2, oy2)) {
        // Collision
        if (obj.type === 'obstacle') {
          lives--;
          if (lives <= 0) {
            lives = 0;
            gameOver = true;
          }
        } else {
          score += 10;
        }
        toRemove.push(i);
      }
    });

    // Remove collided objects
    for (let i = toRemove.length - 1; i >= 0; i--) {
      objects.splice(toRemove[i], 1);
    }
  }

  function rectOverlap(x1,y1,x2,y2, X1,Y1,X2,Y2) {
    return !(x2 < X1 || x1 > X2 || y2 < Y1 || y1 > Y2);
  }

  // Update logic
  function update() {
    if (gameOver) return;

    // Move objects closer
    for (let i = 0; i < objects.length; i++) {
      objects[i].z -= forwardSpeed;
    }

    // Filter out objects that passed behind the player
    objects = objects.filter(o => o.z > -100);

    // Spawn objects
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

    // Check collisions
    checkCollisions();

    // Update HUD
    const hud = document.getElementById('hud');
    hud.innerHTML = `Score: ${score}<br>Lives: ${lives}${gameOver ? '<br><span style="color:#ff0000;">GAME OVER</span>' : ''}`;
  }

  // Draw background
  function drawBackground() {
    const grad = ctx.createLinearGradient(0, 0, 0, H);
    grad.addColorStop(0, '#270031'); 
    grad.addColorStop(0.3, '#ff00ff'); 
    grad.addColorStop(0.6, '#ff0090');
    grad.addColorStop(1, '#001144');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, W, H);

    // Neon sun
    const sunRadius = 80;
    const sunX = W/2;
    const sunY = horizonY - 20;
    const sunGrad = ctx.createRadialGradient(sunX, sunY, sunRadius*0.1, sunX, sunY, sunRadius);
    sunGrad.addColorStop(0, neonYellow);
    sunGrad.addColorStop(1, 'rgba(255,0,255,0)');
    ctx.fillStyle = sunGrad;
    ctx.beginPath();
    ctx.arc(sunX, sunY, sunRadius, 0, Math.PI * 2);
    ctx.fill();
  }

  // Draw road
  function drawRoad() {
    ctx.strokeStyle = neonCyan;
    ctx.lineWidth = 1;
    ctx.setLineDash([5,5]);
    for (let z = 0; z < 2000; z += 200) {
      const {x:xl, y:yl} = projectPoint(-ROAD_WIDTH/2, z);
      const {x:xr, y:yr} = projectPoint( ROAD_WIDTH/2, z);
      ctx.beginPath();
      ctx.moveTo(xl, yl);
      ctx.lineTo(xr, yr);
      ctx.stroke();
    }
    ctx.setLineDash([]);

    // Lane dividers
    ctx.strokeStyle = neonPurple;
    ctx.lineWidth = 2;
    ctx.setLineDash([10,10]);
    for (let l = 1; l < LANE_COUNT; l++) {
      const lx = laneX(l);
      ctx.beginPath();
      const {x:xl1,y:yl1} = projectPoint(lx,0);
      const {x:xl2,y:yl2} = projectPoint(lx,2000);
      ctx.moveTo(xl2, yl2);
      ctx.lineTo(xl1, yl1);
      ctx.stroke();
    }
    ctx.setLineDash([]);
  }

  // Draw car at playerLane
  function drawCar() {
    const playerXOffset = laneX(playerLane);
    // Project player's position to get the horizontal alignment, 
    // but we actually fix the vertical position at bottom.
    const pProj = projectPoint(playerXOffset, 0);
    const carX = pProj.x;
    const carY = H - CAR_HEIGHT - 20; 
    ctx.fillStyle = neonPink;
    ctx.shadowColor = neonPink;
    ctx.shadowBlur = 15;
    ctx.beginPath();
    ctx.rect(carX - CAR_WIDTH/2, carY, CAR_WIDTH, CAR_HEIGHT);
    ctx.fill();
    ctx.shadowBlur = 0;
  }

  // Draw objects
  function drawObjects() {
    objects.forEach(o => {
      const x = laneX(o.lane);
      const {x:ox, y:oy, scale} = projectPoint(x, o.z);
      let ow, oh, color, shape;
      if (o.type==='obstacle') {
        ow = OBSTACLE_WIDTH * scale;
        oh = OBSTACLE_HEIGHT * scale;
        color = neonPurple;
        shape = 'rect';
      } else {
        ow = COIN_SIZE * scale;
        oh = COIN_SIZE * scale;
        color = neonYellow;
        shape = 'circle';
      }
      ctx.fillStyle = color;
      ctx.shadowColor = color;
      ctx.shadowBlur = 10;

      if (shape === 'rect') {
        ctx.fillRect(ox - ow/2, oy - oh, ow, oh);
      } else {
        ctx.beginPath();
        ctx.arc(ox, oy - oh/2, ow/2, 0, Math.PI*2);
        ctx.fill();
      }
      ctx.shadowBlur = 0;
    });
  }

  // Render
  function render() {
    ctx.clearRect(0,0,W,H);
    drawBackground();
    drawRoad();
    drawCar();
    drawObjects();
  }

  // Main loop
  function gameLoop() {
    update();
    render();
    requestAnimationFrame(gameLoop);
  }

  // Start the game
  gameLoop();

})();
