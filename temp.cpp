<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg width="800" height="600" viewBox="0 0 800 600" version="1.1" 
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
  <rect x="0" y="0" width="800" height="600" fill="url(#sunsetGradient)" />

  <!-- Stylized sun -->
  <circle id="sun" cx="400" cy="300" r="80" fill="none" stroke="#FFDD99" stroke-width="4" opacity="0.8">
    <animate attributeName="stroke-width"
             values="4;8;4"
             dur="3s"
             repeatCount="indefinite"/>
  </circle>

  <!-- Horizon lines for a classic retro-wave look -->
  <g stroke="#FF66CC" stroke-width="2" opacity="0.25">
    <line x1="0" y1="370" x2="800" y2="370" />
    <line x1="0" y1="390" x2="800" y2="390" />
    <line x1="0" y1="410" x2="800" y2="410" />
    <line x1="0" y1="430" x2="800" y2="430" />
    <line x1="0" y1="450" x2="800" y2="450" />
    <line x1="0" y1="470" x2="800" y2="470" />
    <line x1="0" y1="490" x2="800" y2="490" />
    <line x1="0" y1="510" x2="800" y2="510" />
    <line x1="0" y1="530" x2="800" y2="530" />
  </g>

  <!-- Foreground (road) -->
  <rect x="0" y="370" width="800" height="230" fill="url(#roadGradient)" />

  <!-- The wireframe grid on the road -->
  <g stroke="#FF00BB" stroke-width="1" opacity="0.2">
    <!-- Vertical lines -->
    <line x1="100" y1="370" x2="0" y2="600"/>
    <line x1="200" y1="370" x2="50" y2="600"/>
    <line x1="300" y1="370" x2="100" y2="600"/>
    <line x1="400" y1="370" x2="150" y2="600"/>
    <line x1="500" y1="370" x2="200" y2="600"/>
    <line x1="600" y1="370" x2="250" y2="600"/>
    <line x1="700" y1="370" x2="300" y2="600"/>
    <!-- Horizontal lines -->
    <line x1="0" y1="400" x2="800" y2="400"/>
    <line x1="0" y1="430" x2="800" y2="430"/>
    <line x1="0" y1="460" x2="800" y2="460"/>
    <line x1="0" y1="490" x2="800" y2="490"/>
    <line x1="0" y1="520" x2="800" y2="520"/>
    <line x1="0" y1="550" x2="800" y2="550"/>
  </g>

  <!-- The Car Group (more quad-like) -->
  <!-- We'll create a blocky shape: a main body rectangle, a cabin block, and front/back shapes less pointy. -->
  <g id="car" transform="translate(-250,420) scale(0.8)">
    <!-- Animate car movement -->
    <animateTransform attributeName="transform"
                      type="translate"
                      from="-250,420"
                      to="1000,420"
                      dur="4s"
                      repeatCount="indefinite" />
    <!-- Car body - a series of rectangles and gentle slopes -->
    <!-- Body Base -->
    <path d="M0 20 H240 V0 H220 V-10 H60 V0 H0 Z"
          fill="url(#carBodyGradient)" stroke="#FFEEFF" stroke-width="2"/>
    <!-- Cabin top (windows) -->
    <path d="M60 -10 H180 V-20 H80 Z"
          fill="url(#windowGradient)" stroke="#8888AA" stroke-width="1"/>
    <!-- Headlights -->
    <rect x="230" y="5" width="5" height="3" fill="#FFDD66"/>
    <rect x="5" y="5" width="5" height="3" fill="#66FFDD"/>

    <!-- Wheels -->
    <!-- We'll separate them into groups to rotate them -->
    <g id="frontWheel">
      <circle cx="190" cy="20" r="10" fill="#111111" stroke="#FFEEFF" stroke-width="2"/>
      <circle cx="190" cy="20" r="5" fill="#FF44AA"/>
      <animateTransform attributeName="transform"
                        type="rotate"
                        from="0 190 20"
                        to="360 190 20"
                        dur="0.7s"
                        repeatCount="indefinite"/>
    </g>
    <g id="rearWheel">
      <circle cx="40" cy="20" r="10" fill="#111111" stroke="#FFEEFF" stroke-width="2"/>
      <circle cx="40" cy="20" r="5" fill="#FF44AA"/>
      <animateTransform attributeName="transform"
                        type="rotate"
                        from="0 40 20"
                        to="360 40 20"
                        dur="0.7s"
                        repeatCount="indefinite"/>
    </g>
  </g>

  <!-- Optional: Some subtle stars or dust for retro ambiance -->
  <g fill="#FFFFFF" opacity="0.5">
    <circle cx="100" cy="100" r="1"/>
    <circle cx="700" cy="150" r="1.5"/>
    <circle cx="500" cy="80" r="2"/>
    <circle cx="600" cy="220" r="1"/>
    <circle cx="250" cy="50" r="1.5"/>
    <circle cx="380" cy="180" r="1"/>
  </g>

</svg>