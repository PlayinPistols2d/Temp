<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg width="1920" height="1080" viewBox="0 0 1920 1080"
     xmlns="http://www.w3.org/2000/svg">

  <!-- Definitions for gradients and patterns -->
  <defs>
    <!-- Retro sunset gradient -->
    <linearGradient id="sunsetGradient" x1="0%" y1="100%" x2="0%" y2="0%">
      <stop offset="0%" style="stop-color:#330044;stop-opacity:1"/>
      <stop offset="25%" style="stop-color:#551166;stop-opacity:1"/>
      <stop offset="50%" style="stop-color:#882288;stop-opacity:1"/>
      <stop offset="75%" style="stop-color:#CC4488;stop-opacity:1"/>
      <stop offset="100%" style="stop-color:#FF5599;stop-opacity:1"/>
    </linearGradient>

    <!-- Road/foreground gradient -->
    <linearGradient id="roadGradient" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" style="stop-color:#222222;stop-opacity:1"/>
      <stop offset="100%" style="stop-color:#000000;stop-opacity:1"/>
    </linearGradient>

    <!-- Car body gradient -->
    <linearGradient id="carBodyGradient" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" style="stop-color:#FF44AA;stop-opacity:1"/>
      <stop offset="100%" style="stop-color:#FF66CC;stop-opacity:1"/>
    </linearGradient>

    <!-- Car window gradient -->
    <linearGradient id="windowGradient" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" style="stop-color:#111122;stop-opacity:1"/>
      <stop offset="100%" style="stop-color:#333355;stop-opacity:1"/>
    </linearGradient>
  </defs>

  <!-- Background sky -->
  <rect x="0" y="0" width="1920" height="1080" fill="url(#sunsetGradient)" />

  <!-- Stylized sun (centered) -->
  <circle id="sun" cx="960" cy="540" r="150" fill="none" stroke="#FFDD99" stroke-width="8" opacity="0.8">
    <animate attributeName="stroke-width"
             values="8;16;8"
             dur="3s"
             repeatCount="indefinite"/>
  </circle>

  <!-- Horizon lines for a classic retro-wave look -->
  <!-- We'll place lines starting near y=700 downwards -->
  <g stroke="#FF66CC" stroke-width="3" opacity="0.25">
    <line x1="0" y1="700" x2="1920" y2="700" />
    <line x1="0" y1="730" x2="1920" y2="730" />
    <line x1="0" y1="760" x2="1920" y2="760" />
    <line x1="0" y1="790" x2="1920" y2="790" />
    <line x1="0" y1="820" x2="1920" y2="820" />
    <line x1="0" y1="850" x2="1920" y2="850" />
    <line x1="0" y1="880" x2="1920" y2="880" />
    <line x1="0" y1="910" x2="1920" y2="910" />
    <line x1="0" y1="940" x2="1920" y2="940" />
    <line x1="0" y1="970" x2="1920" y2="970" />
  </g>

  <!-- Foreground (road) -->
  <rect x="0" y="700" width="1920" height="380" fill="url(#roadGradient)" />

  <!-- The wireframe grid on the road -->
  <g stroke="#FF00BB" stroke-width="1" opacity="0.2">
    <!-- Vertical lines - perspective lines moving downward -->
    <!-- We'll create a perspective by having them meet offscreen below -->
    <line x1="200" y1="700" x2="-200" y2="1080"/>
    <line x1="400" y1="700" x2="0"    y2="1080"/>
    <line x1="600" y1="700" x2="200"  y2="1080"/>
    <line x1="800" y1="700" x2="400"  y2="1080"/>
    <line x1="1000" y1="700" x2="600" y2="1080"/>
    <line x1="1200" y1="700" x2="800" y2="1080"/>
    <line x1="1400" y1="700" x2="1000" y2="1080"/>
    <line x1="1600" y1="700" x2="1200" y2="1080"/>
    <line x1="1800" y1="700" x2="1400" y2="1080"/>

    <!-- Horizontal lines -->
    <line x1="0" y1="730" x2="1920" y2="730"/>
    <line x1="0" y1="760" x2="1920" y2="760"/>
    <line x1="0" y1="790" x2="1920" y2="790"/>
    <line x1="0" y1="820" x2="1920" y2="820"/>
    <line x1="0" y1="850" x2="1920" y2="850"/>
    <line x1="0" y1="880" x2="1920" y2="880"/>
    <line x1="0" y1="910" x2="1920" y2="910"/>
    <line x1="0" y1="940" x2="1920" y2="940"/>
    <line x1="0" y1="970" x2="1920" y2="970"/>
  </g>

  <!-- Palm Trees Silhouette (for extra detail) -->
  <!-- Left Palm -->
  <g fill="none" stroke="#111111" stroke-width="5">
    <!-- Trunk -->
    <path d="M150,700 L160,600 L165,500 L170,400" stroke="#000000"/>
    <!-- Leaves -->
    <path d="M170,400 C130,380,130,430,170,400" />
    <path d="M170,400 C210,380,210,430,170,400" />
    <path d="M170,400 C130,360,100,400,170,400" />
    <path d="M170,400 C210,360,240,400,170,400" />
  </g>

  <!-- Right Palm -->
  <g fill="none" stroke="#111111" stroke-width="5">
    <path d="M1770,700 L1760,600 L1755,500 L1750,400" stroke="#000000"/>
    <path d="M1750,400 C1710,380,1710,430,1750,400" />
    <path d="M1750,400 C1790,380,1790,430,1750,400" />
    <path d="M1750,400 C1710,360,1680,400,1750,400" />
    <path d="M1750,400 C1790,360,1820,400,1750,400" />
  </g>

  <!-- The Car Group (blocky, quad-like), driving from right to left -->
  <g id="car" transform="translate(2100,780) scale(1)">
    <!-- Animate car movement from right (2100) to left (-400) -->
    <animateTransform attributeName="transform"
                      type="translate"
                      from="2100,780"
                      to="-400,780"
                      dur="6s"
                      repeatCount="indefinite" />
    <!-- Car body -->
    <path d="M0 40 H300 V10 H270 V0 H90 V10 H0 Z"
          fill="url(#carBodyGradient)" stroke="#FFEEFF" stroke-width="4"/>
    <!-- Cabin top (windows) -->
    <path d="M90 0 H240 V-30 H100 Z"
          fill="url(#windowGradient)" stroke="#8888AA" stroke-width="2"/>
    <!-- Headlights (front is now on the left as it drives right-to-left, we flip them) -->
    <rect x="5" y="15" width="8" height="5" fill="#FFDD66"/>
    <rect x="290" y="15" width="8" height="5" fill="#66FFDD"/>

    <!-- Wheels -->
    <g id="frontWheel">
      <circle cx="250" cy="40" r="15" fill="#111111" stroke="#FFEEFF" stroke-width="3"/>
      <circle cx="250" cy="40" r="7" fill="#FF44AA"/>
      <animateTransform attributeName="transform"
                        type="rotate"
                        from="0 250 40"
                        to="360 250 40"
                        dur="0.7s"
                        repeatCount="indefinite"/>
    </g>
    <g id="rearWheel">
      <circle cx="60" cy="40" r="15" fill="#111111" stroke="#FFEEFF" stroke-width="3"/>
      <circle cx="60" cy="40" r="7" fill="#FF44AA"/>
      <animateTransform attributeName="transform"
                        type="rotate"
                        from="0 60 40"
                        to="360 60 40"
                        dur="0.7s"
                        repeatCount="indefinite"/>
    </g>
  </g>

  <!-- Stars: animate their opacity to make them glow and shine -->
  <g fill="#FFFFFF">
    <circle cx="300" cy="200" r="2">
      <animate attributeName="opacity" values="1;0.2;1" dur="3s" repeatCount="indefinite"/>
    </circle>
    <circle cx="600" cy="150" r="3">
      <animate attributeName="opacity" values="1;0;1" dur="4s" repeatCount="indefinite"/>
    </circle>
    <circle cx="1200" cy="100" r="2">
      <animate attributeName="opacity" values="1;0.2;1" dur="2s" repeatCount="indefinite"/>
    </circle>
    <circle cx="1700" cy="250" r="2">
      <animate attributeName="opacity" values="1;0.3;1" dur="3s" repeatCount="indefinite"/>
    </circle>
    <circle cx="1400" cy="80" r="3">
      <animate attributeName="opacity" values="1;0;1" dur="5s" repeatCount="indefinite"/>
    </circle>
    <circle cx="900" cy="300" r="2">
      <animate attributeName="opacity" values="1;0.2;1" dur="4s" repeatCount="indefinite"/>
    </circle>
  </g>

</svg>