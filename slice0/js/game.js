/**
 * Studio — Slice 0
 * Tiny playable balcony prototype. Placeholders only.
 * Controls: WASD/Arrows move, Space jump, E interact.
 */
(function () {
  "use strict";

  const canvas = document.getElementById("game");
  const ctx = canvas.getContext("2d");
  const statusEl = document.getElementById("status");
  const toastEl = document.getElementById("toast");

  const W = canvas.width;
  const H = canvas.height;
  const GRAVITY = 0.55;
  const MOVE_SPEED = 3.2;
  const JUMP_V = -11.2;
  const GROUND_FRICTION = 0.82;

  // Palette (Prism mood)
  const C = {
    skyTop: "#9eb8cc",
    skyBot: "#d4c4a8",
    haze: "rgba(200, 210, 220, 0.35)",
    neonBleed: "rgba(120, 80, 200, 0.12)",
    concrete: "#b8a890",
    concreteDark: "#8a7a68",
    railing: "#9a8a78",
    terracotta: "#c4784a",
    soil: "#5c4030",
    herb: "#4a8a4a",
    herbLite: "#6aaa5a",
    tomato: "#c84a3a",
    tomatoLeaf: "#3d7a3a",
    potato: "#8a6a3a",
    wall: "#d2c2a8",
    wallShadow: "#a89880",
    glass: "rgba(160, 200, 220, 0.45)",
    glassFrame: "#6a5a48",
    amberSpill: "rgba(255, 180, 80, 0.35)",
    desk: "#6e5040",
    deskTop: "#8a6848",
    copper: "#b88858",
    player: "#5a6a78",
    playerAccent: "#e8c8a0",
    catBody: "#c89050",
    catStripe: "#8a6038",
    catCream: "#e8d4b0",
    label: "#2a241c",
    labelBg: "rgba(244, 235, 224, 0.85)",
    prompt: "#ffe8c0",
  };

  const keys = Object.create(null);
  let toastTimer = 0;

  function setStatus(msg) {
    statusEl.textContent = msg;
  }

  function toast(msg, ms) {
    toastEl.hidden = false;
    toastEl.textContent = msg;
    toastTimer = ms || 2800;
  }

  // Platforms: player balcony (left), neighbor balcony (right), small gap between
  const platforms = [
    {
      id: "player",
      x: 40,
      y: 380,
      w: 400,
      h: 24,
      label: "YOUR BALCONY",
    },
    {
      id: "neighbor",
      x: 520,
      y: 360,
      w: 400,
      h: 24,
      label: "NEIGHBOR BALCONY",
    },
  ];

  // Interactables
  const interactables = [
    {
      id: "herbs",
      x: 90,
      y: 330,
      w: 70,
      h: 50,
      balcony: "player",
      prompt: "E — check herbs",
      once: false,
      done: false,
      onInteract() {
        toast("Mint and basil — still alive. Someone watered these recently.");
        setStatus("Your herbs are thriving. Neighbor garden might need finishing.");
      },
    },
    {
      id: "desk",
      x: 300,
      y: 300,
      w: 90,
      h: 80,
      balcony: "player",
      prompt: "E — use engineering desk",
      once: false,
      done: false,
      onInteract() {
        toast("Desk stub: tools ready. Crafting tree comes later.");
        setStatus("Engineering desk — placeholder build station. No craft tree yet.");
      },
    },
    {
      id: "garden",
      x: 600,
      y: 300,
      w: 100,
      h: 60,
      balcony: "neighbor",
      prompt: "E — inspect potato/tomato bed",
      once: false,
      done: false,
      onInteract() {
        toast("Half-started garden: potatoes in soil, tomato stakes waiting.");
        setStatus("Neighbor started this garden. You can finish it later.");
      },
    },
    {
      id: "cat",
      x: 780,
      y: 280,
      w: 70,
      h: 80,
      balcony: "neighbor",
      prompt: "E — open glass / free the cat",
      once: true,
      done: false,
      onInteract() {
        if (this.done) {
          toast("The cat rubs against your leg. Adopted.");
          return;
        }
        this.done = true;
        this.prompt = "E — pet the cat";
        this.x = 530;
        this.y = 300;
        this.w = 70;
        this.h = 60;
        state.catFreed = true;
        toast("You open the glass. The cat steps out — free, and yours.");
        setStatus("Cat freed and adopted. Warmth on the balcony.");
      },
    },
  ];

  const player = {
    x: 180,
    y: 300,
    w: 28,
    h: 48,
    vx: 0,
    vy: 0,
    onGround: false,
    facing: 1,
  };

  const state = {
    catFreed: false,
    near: null,
  };

  window.addEventListener("keydown", (e) => {
    const k = e.key.toLowerCase();
    if (
      [
        "arrowleft",
        "arrowright",
        "arrowup",
        "arrowdown",
        " ",
        "w",
        "a",
        "s",
        "d",
        "e",
      ].includes(k) ||
      e.code === "Space"
    ) {
      e.preventDefault();
    }
    keys[e.code] = true;
    keys[k] = true;
    if (k === "e" || e.code === "KeyE") {
      tryInteract();
    }
  });

  window.addEventListener("keyup", (e) => {
    const k = e.key.toLowerCase();
    keys[e.code] = false;
    keys[k] = false;
  });

  function pressed(codes) {
    return codes.some((c) => keys[c]);
  }

  function rectsOverlap(a, b) {
    return (
      a.x < b.x + b.w &&
      a.x + a.w > b.x &&
      a.y < b.y + b.h &&
      a.y + a.h > b.y
    );
  }

  function tryInteract() {
    if (!state.near) {
      toast("Nothing to interact with here.");
      return;
    }
    state.near.onInteract();
  }

  function updateNear() {
    const pad = 18;
    const zone = {
      x: player.x - pad,
      y: player.y - pad,
      w: player.w + pad * 2,
      h: player.h + pad * 2,
    };
    state.near = null;
    for (const it of interactables) {
      if (rectsOverlap(zone, it)) {
        state.near = it;
        break;
      }
    }
  }

  function resolvePlatforms() {
    player.onGround = false;
    for (const p of platforms) {
      const prevBottom = player.y + player.h - player.vy;
      const landing =
        player.vy >= 0 &&
        prevBottom <= p.y + 4 &&
        player.x + player.w > p.x + 4 &&
        player.x < p.x + p.w - 4 &&
        player.y + player.h >= p.y &&
        player.y + player.h <= p.y + p.h + 12;

      if (landing) {
        player.y = p.y - player.h;
        player.vy = 0;
        player.onGround = true;
      }
    }
  }

  function update() {
    const left = pressed(["ArrowLeft", "a", "KeyA"]);
    const right = pressed(["ArrowRight", "d", "KeyD"]);
    const jump = pressed([" ", "Space", "ArrowUp", "w", "KeyW"]);

    if (left) {
      player.vx = -MOVE_SPEED;
      player.facing = -1;
    } else if (right) {
      player.vx = MOVE_SPEED;
      player.facing = 1;
    } else {
      player.vx *= GROUND_FRICTION;
      if (Math.abs(player.vx) < 0.1) player.vx = 0;
    }

    if (jump && player.onGround) {
      player.vy = JUMP_V;
      player.onGround = false;
    }

    player.vy += GRAVITY;
    player.x += player.vx;
    player.y += player.vy;

    // World bounds (soft)
    if (player.x < 10) player.x = 10;
    if (player.x + player.w > W - 10) player.x = W - 10 - player.w;

    // Fall reset
    if (player.y > H + 80) {
      player.x = 180;
      player.y = 300;
      player.vx = 0;
      player.vy = 0;
      toast("You scramble back onto your balcony.");
    }

    resolvePlatforms();
    updateNear();

    if (toastTimer > 0) {
      toastTimer -= 16;
      if (toastTimer <= 0) toastEl.hidden = true;
    }
  }

  function drawSky() {
    const g = ctx.createLinearGradient(0, 0, 0, H);
    g.addColorStop(0, C.skyTop);
    g.addColorStop(0.55, C.skyBot);
    g.addColorStop(1, "#c8b898");
    ctx.fillStyle = g;
    ctx.fillRect(0, 0, W, H);

    // Soft haze bands
    ctx.fillStyle = C.haze;
    ctx.fillRect(0, 80, W, 40);
    ctx.fillRect(0, 160, W, 28);

    // Distant neon bleed (optional, subtle)
    ctx.fillStyle = C.neonBleed;
    ctx.beginPath();
    ctx.ellipse(860, 120, 90, 30, 0, 0, Math.PI * 2);
    ctx.fill();
  }

  function drawBuildingWalls() {
    // Player apartment wall (left)
    ctx.fillStyle = C.wall;
    ctx.fillRect(40, 200, 160, 180);
    ctx.fillStyle = C.wallShadow;
    ctx.fillRect(40, 200, 12, 180);

    // Desk interior stub / amber spill
    ctx.fillStyle = C.amberSpill;
    ctx.fillRect(52, 250, 100, 100);
    ctx.fillStyle = C.desk;
    ctx.fillRect(300, 332, 90, 48);
    ctx.fillStyle = C.deskTop;
    ctx.fillRect(295, 324, 100, 12);
    // Tool clutter
    ctx.fillStyle = C.copper;
    ctx.fillRect(310, 312, 18, 12);
    ctx.fillRect(340, 308, 8, 16);
    ctx.fillRect(360, 314, 22, 8);

    // Neighbor wall + glass door
    ctx.fillStyle = C.wall;
    ctx.fillRect(760, 180, 160, 180);
    ctx.fillStyle = C.glassFrame;
    ctx.fillRect(780, 220, 80, 140);
    ctx.fillStyle = C.glass;
    ctx.fillRect(786, 226, 68, 128);
    // Amber interior spill behind glass
    ctx.fillStyle = C.amberSpill;
    ctx.fillRect(790, 240, 50, 90);
    // Paw prints hint on glass
    ctx.fillStyle = "rgba(90, 70, 50, 0.35)";
    for (let i = 0; i < 3; i++) {
      ctx.beginPath();
      ctx.ellipse(810 + i * 12, 320 - i * 8, 5, 4, 0, 0, Math.PI * 2);
      ctx.fill();
    }
  }

  function drawPlatform(p) {
    // Slab
    ctx.fillStyle = C.concrete;
    ctx.fillRect(p.x, p.y, p.w, p.h);
    ctx.fillStyle = C.concreteDark;
    ctx.fillRect(p.x, p.y + p.h - 6, p.w, 6);

    // Railing
    ctx.strokeStyle = C.railing;
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(p.x, p.y - 36);
    ctx.lineTo(p.x + p.w, p.y - 36);
    ctx.stroke();
    for (let x = p.x + 16; x < p.x + p.w; x += 36) {
      ctx.beginPath();
      ctx.moveTo(x, p.y - 36);
      ctx.lineTo(x, p.y);
      ctx.stroke();
    }

    // Label
    ctx.font = "bold 13px Segoe UI, sans-serif";
    const tw = ctx.measureText(p.label).width;
    const lx = p.x + (p.w - tw) / 2 - 6;
    const ly = p.y + 48;
    ctx.fillStyle = C.labelBg;
    ctx.fillRect(lx, ly - 14, tw + 12, 20);
    ctx.fillStyle = C.label;
    ctx.fillText(p.label, lx + 6, ly);
  }

  function drawHerbs() {
    const baseX = 100;
    const baseY = 380;
    for (let i = 0; i < 3; i++) {
      const x = baseX + i * 28;
      ctx.fillStyle = C.terracotta;
      ctx.fillRect(x, baseY - 28, 22, 18);
      ctx.fillStyle = C.soil;
      ctx.fillRect(x + 2, baseY - 26, 18, 8);
      ctx.fillStyle = i % 2 ? C.herb : C.herbLite;
      ctx.beginPath();
      ctx.ellipse(x + 11, baseY - 40, 10, 16, 0, 0, Math.PI * 2);
      ctx.fill();
    }
    ctx.font = "11px Segoe UI, sans-serif";
    ctx.fillStyle = C.label;
    ctx.fillText("HERBS", 108, 368);
  }

  function drawGarden() {
    const x = 600;
    const y = 360;
    ctx.fillStyle = C.terracotta;
    ctx.fillRect(x, y - 40, 100, 28);
    ctx.fillStyle = C.soil;
    ctx.fillRect(x + 4, y - 36, 92, 16);
    // Potato mounds
    ctx.fillStyle = C.potato;
    ctx.beginPath();
    ctx.ellipse(x + 25, y - 28, 14, 8, 0, 0, Math.PI * 2);
    ctx.ellipse(x + 50, y - 26, 12, 7, 0, 0, Math.PI * 2);
    ctx.fill();
    // Tomato plant
    ctx.fillStyle = C.tomatoLeaf;
    ctx.fillRect(x + 72, y - 70, 6, 40);
    ctx.beginPath();
    ctx.ellipse(x + 75, y - 72, 16, 12, 0, 0, Math.PI * 2);
    ctx.fill();
    ctx.fillStyle = C.tomato;
    ctx.beginPath();
    ctx.arc(x + 70, y - 55, 5, 0, Math.PI * 2);
    ctx.arc(x + 82, y - 50, 4, 0, Math.PI * 2);
    ctx.fill();
    ctx.font = "11px Segoe UI, sans-serif";
    ctx.fillStyle = C.label;
    ctx.fillText("POTATO / TOMATO", x + 8, y - 48);
  }

  function drawCat() {
    const it = interactables.find((i) => i.id === "cat");
    let cx = it.x + 28;
    let cy = it.y + 50;
    if (state.catFreed) {
      // Sit on neighbor balcony near player-facing edge
      cx = 560;
      cy = 330;
    }

    // Body
    ctx.fillStyle = C.catBody;
    ctx.beginPath();
    ctx.ellipse(cx, cy, 22, 14, 0, 0, Math.PI * 2);
    ctx.fill();
    // Head
    ctx.beginPath();
    ctx.arc(cx + 18, cy - 10, 12, 0, Math.PI * 2);
    ctx.fill();
    // Ears
    ctx.beginPath();
    ctx.moveTo(cx + 10, cy - 18);
    ctx.lineTo(cx + 14, cy - 28);
    ctx.lineTo(cx + 18, cy - 16);
    ctx.moveTo(cx + 20, cy - 16);
    ctx.lineTo(cx + 26, cy - 28);
    ctx.lineTo(cx + 28, cy - 14);
    ctx.fill();
    // Cream belly / muzzle
    ctx.fillStyle = C.catCream;
    ctx.beginPath();
    ctx.ellipse(cx - 2, cy + 4, 10, 8, 0, 0, Math.PI * 2);
    ctx.arc(cx + 20, cy - 8, 5, 0, Math.PI * 2);
    ctx.fill();
    // Stripes
    ctx.strokeStyle = C.catStripe;
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(cx - 10, cy - 4);
    ctx.lineTo(cx - 4, cy + 6);
    ctx.moveTo(cx, cy - 6);
    ctx.lineTo(cx + 4, cy + 6);
    ctx.stroke();
    // Tail
    ctx.strokeStyle = C.catBody;
    ctx.lineWidth = 5;
    ctx.lineCap = "round";
    ctx.beginPath();
    ctx.moveTo(cx - 20, cy);
    ctx.quadraticCurveTo(cx - 36, cy - 20, cx - 28, cy - 32);
    ctx.stroke();

    if (!state.catFreed) {
      ctx.font = "11px Segoe UI, sans-serif";
      ctx.fillStyle = C.label;
      ctx.fillText("CAT (at glass)", cx - 30, cy - 40);
    } else {
      ctx.font = "11px Segoe UI, sans-serif";
      ctx.fillStyle = C.label;
      ctx.fillText("YOUR CAT", cx - 24, cy - 36);
    }
  }

  function drawPlayer() {
    const x = player.x;
    const y = player.y;
    // Body
    ctx.fillStyle = C.player;
    ctx.fillRect(x + 4, y + 14, 20, 28);
    // Head
    ctx.fillStyle = C.playerAccent;
    ctx.beginPath();
    ctx.arc(x + 14, y + 10, 10, 0, Math.PI * 2);
    ctx.fill();
    // Facing marker
    ctx.fillStyle = C.copper;
    const fx = player.facing > 0 ? x + 22 : x + 2;
    ctx.fillRect(fx, y + 22, 6, 4);
    // Legs
    ctx.fillStyle = C.player;
    ctx.fillRect(x + 6, y + 40, 7, 8);
    ctx.fillRect(x + 16, y + 40, 7, 8);

    ctx.font = "11px Segoe UI, sans-serif";
    ctx.fillStyle = C.labelBg;
    ctx.fillRect(x - 4, y - 18, 52, 16);
    ctx.fillStyle = C.label;
    ctx.fillText("INTERN", x, y - 6);
  }

  function drawPrompt() {
    if (!state.near) return;
    const msg = state.near.prompt;
    ctx.font = "bold 14px Segoe UI, sans-serif";
    const tw = ctx.measureText(msg).width;
    const px = Math.max(12, Math.min(W - tw - 24, player.x + player.w / 2 - tw / 2));
    const py = player.y - 36;
    ctx.fillStyle = "rgba(40, 30, 20, 0.85)";
    ctx.fillRect(px - 8, py - 16, tw + 16, 24);
    ctx.strokeStyle = C.copper;
    ctx.lineWidth = 1.5;
    ctx.strokeRect(px - 8, py - 16, tw + 16, 24);
    ctx.fillStyle = C.prompt;
    ctx.fillText(msg, px, py);
  }

  function drawGapHint() {
    ctx.font = "12px Segoe UI, sans-serif";
    ctx.fillStyle = "rgba(40, 30, 20, 0.7)";
    ctx.fillText("jump the gap →", 430, 340);
  }

  function drawFireEscapeHint() {
    // Visual stub only — no street roam
    ctx.strokeStyle = C.copper;
    ctx.lineWidth = 2;
    ctx.setLineDash([4, 4]);
    ctx.beginPath();
    ctx.moveTo(920, 360);
    ctx.lineTo(940, 280);
    ctx.lineTo(920, 220);
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.font = "10px Segoe UI, sans-serif";
    ctx.fillStyle = C.label;
    ctx.fillText("fire escape", 880, 210);
    ctx.fillText("(later)", 895, 222);
  }

  function drawHelpOverlay() {
    ctx.font = "12px Segoe UI, sans-serif";
    ctx.fillStyle = "rgba(40, 32, 24, 0.55)";
    ctx.fillRect(12, 12, 220, 54);
    ctx.fillStyle = "#f4ebe0";
    ctx.fillText("WASD / Arrows move · Space jump", 20, 32);
    ctx.fillText("E interact · two balconies only", 20, 50);
  }

  function draw() {
    drawSky();
    drawBuildingWalls();
    drawFireEscapeHint();
    for (const p of platforms) drawPlatform(p);
    drawHerbs();
    drawGarden();
    drawCat();
    drawGapHint();
    drawPlayer();
    drawPrompt();
    drawHelpOverlay();
  }

  function loop() {
    update();
    draw();
    requestAnimationFrame(loop);
  }

  setStatus("Quiet after the crowd fled. Jump to the neighbor balcony — a cat is at the glass.");
  loop();
})();
