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

  // Perspective/Camera
  const horizonY = H * 0.4;          // horizon line position
  const fov = 300;                   // field of view for perspective
  const playerZ = 0;                 // player is at z=0, objects come towards this point
  const laneWidth = ROAD_WIDTH / LANE_COUNT;
  const laneOffsets = [-laneWidth, 0, laneWidth]; // lanes centered around x=0

  // Game State
  let score = 0;
  let lives = 3;
  let gameOver = false;

  // Player position in x-lanes (0=left,1=middle,2=right)
  let playerLane = 1;

  // Speed at which we move forward (Z-direction)
  let forwardSpeed = 10;

  // Arrays for obstacles and coins (each object: {lane: number, z: number, type:'obstacle'|'coin'})
  let objects = [];

  // Neon Color Palette
  const neonPurple = '#ff00ff';
  const neonCyan = '#00ffff';
  const neonPink = '#ff0090';
  const neonYellow = '#ffff00';

  // Input Controls
  let moveLeft = false;
  let moveRight = false;

  // Periodic spawns
  let obstacleTimer = 0;
  let coinTimer = 0;
  const obstacleSpawnInterval = 100;   // frames between obstacle spawns
  const coinSpawnInterval = 150;       // frames between coin spawns
  const initialSpawnDistance = 2000;   // where new objects spawn in Z distance

  // Input events
  document.addEventListener('keydown', e => {
    if ((e.key === 'ArrowLeft' || e.key === 'a') && playerLane > 0) {
      playerLane -= 1;
    }
    if ((e.key === 'ArrowRight' || e.key === 'd') && playerLane < LANE_COUNT - 1) {
      playerLane += 1;
    }
  });

  // Helper: Project (x,z) to screen coordinates
  // We assume player at (x=0,z=0). Objects have some lane offset (x) and a z distance.
  // scale = fov/(fov+z)
  // screenX = centerX + x*scale
  // screenY = horizonY + (H - horizonY)*scale
  function projectPoint(x, z) {
    const scale = fov / (fov + z);
    const screenX = (W/2) + x * scale;
    const screenY = horizonY + (H - horizonY)*scale;
    return {x: screenX, y: screenY, scale: scale};
  }

  // Lane X offset
  function laneX(laneIndex) {
    return laneOffsets[laneIndex];
  }

  // Spawn an obstacle
  function spawnObstacle() {
    const laneIndex = Math.floor(Math.random() * LANE_COUNT);
    objects.push({ lane: laneIndex, z: initialSpawnDistance, type:'obstacle' });
  }

  // Spawn a coin
  function spawnCoin() {
    const laneIndex = Math.floor(Math.random() * LANE_COUNT);
    objects.push({ lane: laneIndex, z: initialSpawnDistance, type:'coin' });
  }

  // Collision check in screen space:
  // We'll approximate by checking if object's projected position near player's car position overlaps.
  // Player always at bottom. We'll define player position in projection:
  function checkCollisions() {
    // Project player at z=0, x=0
    // Player's "hitbox" in projected space depends on scale= fov/(fov+0)=fov/fov=1
    // Player bottom center is at: (W/2, bottom)
    // Player rect: x=(W/2 - CAR_WIDTH/2, W/2 + CAR_WIDTH/2), y=(H-CAR_HEIGHT, H)
    const playerScreenX = W/2;
    const playerScreenY = H - CAR_HEIGHT;
    const playerHalfW = CAR_WIDTH/2;
    const playerH = CAR_HEIGHT;

    // For each object, project and check overlap
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

      // Object center at ox, oy - let's center rect at that point
      const ox1 = ox - ow/2;
      const oy1 = oy - oh;
      const ox2 = ox + ow/2;
      const oy2 = oy;

      const playerRect = {x1: playerScreenX - playerHalfW, y1: playerScreenY, x2: playerScreenX + playerHalfW, y2: playerScreenY+playerH};
      const objRect = {x1: ox1, y1: oy1, x2: ox2, y2: oy2};

      // Check overlap
      if (rectOverlap(playerRect, objRect)) {
        if (obj.type === 'obstacle') {
          lives--;
          if (lives <= 0) {
            lives = 0;
            gameOver = true;
          }
        } else {
          score += 10;
        }
        objects.splice(i,1);
      }
    });
  }

  function rectOverlap(r1, r2) {
    return !(r1.x2 < r2.x1 || r1.x1 > r2.x2 || r1.y2 < r2.y1 || r1.y1 > r2.y2);
  }

  // Update logic
  function update() {
    if (gameOver) return;

    // Move forward: decrease z of objects by forwardSpeed
    for (let i = 0; i < objects.length; i++) {
      objects[i].z -= forwardSpeed;
    }

    // Remove objects that passed the player (z< -100 for a bit safety)
    objects = objects.filter(o => o.z > -100);

    // Spawn new objects periodically
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

    checkCollisions();

    // Update HUD
    const hud = document.getElementById('hud');
    hud.innerHTML = `Score: ${score}<br>Lives: ${lives}${gameOver ? '<br><span style="color:#ff0000;">GAME OVER</span>' : ''}`;
  }

  // Draw Background (Neon Sunset)
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

  // Draw road and lane lines
  function drawRoad() {
    // We'll draw a series of lines to represent the road
    // The road center x=0, width=ROAD_WIDTH
    // Let's draw the left and right boundaries and lane dividers
    const roadLeftX = -ROAD_WIDTH/2;
    const roadRightX = ROAD_WIDTH/2;

    // Draw road surface (just a big trapezoid blending into horizon)
    // We'll draw multiple horizontal lines to create a neon grid effect
    ctx.strokeStyle = neonCyan;
    ctx.lineWidth = 1;
    ctx.setLineDash([5,5]);
    for (let z = 0; z < 2000; z += 200) {
      const {x:xl, y:yl} = projectPoint(roadLeftX, z);
      const {x:xr, y:yr} = projectPoint(roadRightX, z);
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
      const lx = laneX(l) ;
      ctx.beginPath();
      const {x:xl1,y:yl1} = projectPoint(lx,0);
      const {x:xl2,y:yl2} = projectPoint(lx,2000);
      ctx.moveTo(xl2, yl2);
      ctx.lineTo(xl1, yl1);
      ctx.stroke();
    }
    ctx.setLineDash([]);
  }

  // Draw Car (player) at bottom center
  function drawCar() {
    ctx.fillStyle = neonPink;
    ctx.shadowColor = neonPink;
    ctx.shadowBlur = 15;

    const playerX = W/2; 
    const playerY = H - CAR_HEIGHT - 20; // a bit above bottom
    ctx.beginPath();
    ctx.rect(playerX - CAR_WIDTH/2, playerY, CAR_WIDTH, CAR_HEIGHT);
    ctx.fill();

    ctx.shadowBlur = 0;
  }

  // Draw objects (obstacles and coins)
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

  // Start
  gameLoop();
})();