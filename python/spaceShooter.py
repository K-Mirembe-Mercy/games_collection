"""
╔══════════════════════════════════════════════════════════╗
║          GALACTIC FURY 3D  —  900+ LEVELS                ║
║  Requirements: pip install pygame PyOpenGL PyOpenGL_accelerate numpy  ║
╠══════════════════════════════════════════════════════════╣
║  CONTROLS                                                ║
║  WASD / Arrows   Move ship                               ║
║  Space           Fire primary                            ║
║  LShift          Fire missiles (when charged)            ║
║  Q/E             Roll left / right                       ║
║  P               Pause                                   ║
║  ESC             Menu / Quit                             ║
╚══════════════════════════════════════════════════════════╝
"""

import pygame
import sys, math, random, time, struct, array, os

# ── Try OpenGL ─────────────────────────────────────────────────────────────
try:
    from OpenGL.GL import *
    from OpenGL.GLU import *
    OPENGL = True
except ImportError:
    OPENGL = False
    print("[WARN] PyOpenGL not found – falling back to 2.5D software renderer")

import numpy as np

# ══════════════════════════════════════════════════════════════════════════════
#  CONSTANTS
# ══════════════════════════════════════════════════════════════════════════════
W, H        = 1024, 768
FOV         = 60
NEAR, FAR   = 0.1, 2000.0
FPS         = 60
MAX_LEVELS  = 900

# colour palette
C_BG        = (0.02, 0.02, 0.08, 1)
C_CYAN      = (0.0,  0.9,  1.0)
C_RED       = (1.0,  0.15, 0.1)
C_ORANGE    = (1.0,  0.55, 0.0)
C_YELLOW    = (1.0,  0.9,  0.0)
C_GREEN     = (0.1,  1.0,  0.4)
C_PURPLE    = (0.7,  0.1,  1.0)
C_WHITE     = (1.0,  1.0,  1.0)
C_PINK      = (1.0,  0.3,  0.7)
C_BLUE      = (0.2,  0.4,  1.0)

pygame.init()
pygame.mixer.init(frequency=22050, size=-16, channels=2, buffer=256)

# ══════════════════════════════════════════════════════════════════════════════
#  PROCEDURAL SOUND
# ══════════════════════════════════════════════════════════════════════════════
def make_sound(freq=440, dur=0.10, vol=0.28, shape="sq", decay=True, vibrato=0):
    rate = 22050
    n    = int(rate * dur)
    buf  = np.zeros(n, dtype=np.float32)
    for i in range(n):
        t   = i / rate
        f   = freq + vibrato * math.sin(2*math.pi*5*t)
        env = (1 - i/n)**1.5 if decay else 1.0
        if shape == "sq":
            v = 1.0 if math.sin(2*math.pi*f*t) > 0 else -1.0
        elif shape == "noise":
            v = random.uniform(-1, 1)
        elif shape == "saw":
            v = 2*(f*t % 1) - 1
        else:
            v = math.sin(2*math.pi*f*t)
        buf[i] = v * env * vol
    pcm = (buf * 32767).astype(np.int16)
    snd = pygame.sndarray.make_sound(np.column_stack([pcm, pcm]))
    return snd

try:
    SFX = {
        "shoot":   make_sound(700,  0.07, 0.22, "sq"),
        "shoot2":  make_sound(400,  0.12, 0.25, "saw"),
        "explode": make_sound(80,   0.40, 0.38, "noise"),
        "hit":     make_sound(220,  0.09, 0.28, "sq"),
        "power":   make_sound(880,  0.22, 0.30, "sine", vibrato=40),
        "level":   make_sound(660,  0.35, 0.32, "sine", vibrato=20),
        "missile": make_sound(300,  0.18, 0.30, "saw"),
        "shield":  make_sound(440,  0.15, 0.20, "sine"),
        "boss":    make_sound(120,  0.50, 0.35, "sq"),
    }
    SFX_OK = True
except:
    SFX_OK = False

def sfx(name):
    if SFX_OK:
        try: SFX[name].play()
        except: pass

# ══════════════════════════════════════════════════════════════════════════════
#  MATH HELPERS
# ══════════════════════════════════════════════════════════════════════════════
def v3(x=0,y=0,z=0): return np.array([x,y,z], dtype=np.float32)
def v3len(v):         return float(np.linalg.norm(v))
def v3norm(v):
    l = v3len(v)
    return v/l if l > 1e-9 else v3()
def v3dist(a,b):      return v3len(b-a)
def lerp(a,b,t):      return a + (b-a)*t
def clamp(v,lo,hi):   return max(lo, min(hi, v))
def hsv2rgb(h,s,v):
    import colorsys
    return colorsys.hsv_to_rgb(h,s,v)

# ══════════════════════════════════════════════════════════════════════════════
#  LEVEL SYSTEM  (900 levels, procedurally defined)
# ══════════════════════════════════════════════════════════════════════════════
ZONES = [
    # (name, sky_hue, fog_density, bg_r,bg_g,bg_b)
    ("Asteroid Belt",   0.55, 0.004, 0.02,0.02,0.08),
    ("Nebula Drift",    0.75, 0.003, 0.05,0.01,0.10),
    ("Solar Flare",     0.08, 0.005, 0.12,0.03,0.02),
    ("Ice Corridor",    0.50, 0.002, 0.01,0.05,0.12),
    ("Dark Matter",     0.85, 0.008, 0.01,0.00,0.06),
    ("Binary Star",     0.12, 0.004, 0.10,0.06,0.01),
    ("Void Gate",       0.60, 0.006, 0.00,0.02,0.10),
    ("Quasar Storm",    0.30, 0.007, 0.08,0.02,0.00),
    ("Crystal Expanse", 0.45, 0.003, 0.02,0.08,0.10),
]

ENEMY_CONFIGS = [
    # tier, (r,g,b), hp_mult, speed_mult, score, shoot_rate
    (1, C_RED,    1.0, 1.0, 10,  120),
    (2, C_ORANGE, 1.5, 0.9, 25,  100),
    (3, C_PURPLE, 2.5, 0.8, 60,   80),
    (4, C_PINK,   4.0, 1.2, 100,  60),
    (5, C_YELLOW, 6.0, 1.5, 200,  45),  # elite
]
BOSS_CONFIGS = [
    (C_RED,    15,  1, 500),
    (C_PURPLE, 25,  2, 1000),
    (C_YELLOW, 40,  3, 2000),
    (C_PINK,   60,  4, 4000),
    (C_WHITE,  100, 5, 8000),
]

def level_config(lvl):
    """Returns a dict describing everything about a level."""
    z        = ZONES[(lvl-1) // 100 % len(ZONES)]
    segment  = (lvl - 1) % 100          # 0-99 within zone
    boss_lvl = (segment == 99)          # every 100th is a boss
    tier_cap = clamp((lvl-1)//50 + 1, 1, 5)
    n_waves  = clamp(2 + lvl//30, 2, 12)
    enemies_per_wave = clamp(4 + lvl//10, 4, 30)
    speed_mult = 1.0 + lvl * 0.004
    hp_mult    = 1.0 + lvl * 0.008
    formation  = ["scatter","v","diamond","ring","cross","spiral"][lvl % 6]
    powerup_chance = clamp(0.15 + lvl * 0.0002, 0.15, 0.35)
    boss_idx   = clamp((lvl-1)//180, 0, len(BOSS_CONFIGS)-1)
    asteroid_density = clamp(lvl//60, 0, 5)
    return dict(
        level=lvl, zone=z, boss=boss_lvl,
        tier_cap=tier_cap, n_waves=n_waves,
        enemies_per_wave=enemies_per_wave,
        speed_mult=speed_mult, hp_mult=hp_mult,
        formation=formation, powerup_chance=powerup_chance,
        boss_idx=boss_idx, asteroid_density=asteroid_density,
    )

# ══════════════════════════════════════════════════════════════════════════════
#  3D RENDERER  (OpenGL or software fallback)
# ══════════════════════════════════════════════════════════════════════════════
class Renderer:
    def __init__(self, surf):
        self.surf = surf
        self.use_gl = OPENGL

    # ── OpenGL helpers ──────────────────────────────────────────────────────
    def begin_frame(self, bg):
        if self.use_gl:
            glClearColor(*bg, 1)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    def set_3d(self):
        if not self.use_gl: return
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(FOV, W/H, NEAR, FAR)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def set_2d(self):
        if not self.use_gl: return
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(0, W, H, 0, -1, 1)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        glDisable(GL_DEPTH_TEST)

    # ── Draw 3D wireframe box ───────────────────────────────────────────────
    def wire_box(self, pos, size, col, alpha=1.0, rot_y=0, rot_x=0):
        if not self.use_gl: return
        glPushMatrix()
        glTranslatef(*pos)
        glRotatef(rot_y, 0, 1, 0)
        glRotatef(rot_x, 1, 0, 0)
        glColor4f(*col, alpha)
        glLineWidth(1.5)
        s = size/2
        verts = [(-s,-s,-s),(s,-s,-s),(s,s,-s),(-s,s,-s),
                 (-s,-s, s),(s,-s, s),(s,s, s),(-s,s, s)]
        edges = [(0,1),(1,2),(2,3),(3,0),(4,5),(5,6),(6,7),(7,4),
                 (0,4),(1,5),(2,6),(3,7)]
        glBegin(GL_LINES)
        for e in edges:
            for vi in e:
                glVertex3fv(verts[vi])
        glEnd()
        glPopMatrix()

    def solid_box(self, pos, size, col, alpha=1.0, rot_y=0, rot_x=0):
        if not self.use_gl: return
        glPushMatrix()
        glTranslatef(*pos)
        glRotatef(rot_y, 0, 1, 0)
        glRotatef(rot_x, 1, 0, 0)
        glColor4f(*col, alpha)
        s = size/2
        faces = [
            [(-s,-s,-s),(s,-s,-s),(s,s,-s),(-s,s,-s)],
            [(-s,-s, s),(s,-s, s),(s,s, s),(-s,s, s)],
            [(-s,-s,-s),(-s,-s, s),(-s,s, s),(-s,s,-s)],
            [(s,-s,-s),(s,-s, s),(s,s, s),(s,s,-s)],
            [(-s,-s,-s),(s,-s,-s),(s,-s, s),(-s,-s, s)],
            [(-s,s,-s),(s,s,-s),(s,s, s),(-s,s, s)],
        ]
        normals = [(0,0,-1),(0,0,1),(-1,0,0),(1,0,0),(0,-1,0),(0,1,0)]
        glBegin(GL_QUADS)
        for f, n in zip(faces, normals):
            glNormal3fv(n)
            for v in f: glVertex3fv(v)
        glEnd()
        glPopMatrix()

    def draw_sphere(self, pos, r, col, alpha=1.0, slices=8):
        if not self.use_gl: return
        glPushMatrix()
        glTranslatef(*pos)
        glColor4f(*col, alpha)
        q = gluNewQuadric()
        gluQuadricDrawStyle(q, GLU_LINE)
        gluSphere(q, r, slices, slices//2)
        gluDeleteQuadric(q)
        glPopMatrix()

    def draw_cone(self, pos, r, h, col, alpha=1.0, rot_y=0):
        if not self.use_gl: return
        glPushMatrix()
        glTranslatef(*pos)
        glRotatef(rot_y, 0, 1, 0)
        glRotatef(-90, 1, 0, 0)
        glColor4f(*col, alpha)
        q = gluNewQuadric()
        gluQuadricDrawStyle(q, GLU_LINE)
        gluCylinder(q, 0, r, h, 8, 1)
        gluDeleteQuadric(q)
        glPopMatrix()

    def draw_line(self, a, b, col, alpha=1.0, width=1.0):
        if not self.use_gl: return
        glLineWidth(width)
        glColor4f(*col, alpha)
        glBegin(GL_LINES)
        glVertex3fv(a); glVertex3fv(b)
        glEnd()

    def draw_torus(self, pos, R, r, col, alpha=1.0, rot_y=0):
        if not self.use_gl: return
        glPushMatrix()
        glTranslatef(*pos)
        glRotatef(rot_y, 0, 1, 0)
        glColor4f(*col, alpha)
        segs, sides = 16, 8
        glBegin(GL_LINES)
        for i in range(segs):
            a0 = 2*math.pi*i/segs
            a1 = 2*math.pi*(i+1)/segs
            for j in range(sides):
                b0 = 2*math.pi*j/sides
                b1 = 2*math.pi*(j+1)/sides
                def pt(a,b):
                    return ((R+r*math.cos(b))*math.cos(a),
                            r*math.sin(b),
                            (R+r*math.cos(b))*math.sin(a))
                glVertex3fv(pt(a0,b0)); glVertex3fv(pt(a1,b0))
                glVertex3fv(pt(a0,b0)); glVertex3fv(pt(a0,b1))
        glEnd()
        glPopMatrix()

    # ── Particle billboard (quad) ───────────────────────────────────────────
    def draw_particle_gl(self, pos, size, col, alpha):
        if not self.use_gl: return
        s = size/2
        glColor4f(*col, alpha)
        glBegin(GL_QUADS)
        glVertex3f(pos[0]-s, pos[1]-s, pos[2])
        glVertex3f(pos[0]+s, pos[1]-s, pos[2])
        glVertex3f(pos[0]+s, pos[1]+s, pos[2])
        glVertex3f(pos[0]-s, pos[1]+s, pos[2])
        glEnd()

    # ── 2D text overlay via pygame surface ──────────────────────────────────
    def blit_text(self, text, x, y, col=(255,255,255), font=None):
        if font is None:
            font = pygame.font.SysFont("Consolas", 20, bold=True)
        surf = font.render(text, True, col)
        if self.use_gl:
            # blit onto the display surface in 2D mode
            data = pygame.image.tostring(surf, "RGBA", True)
            gw, gh = surf.get_size()
            glWindowPos2i(x, H - y - gh)
            glDrawPixels(gw, gh, GL_RGBA, GL_UNSIGNED_BYTE, data)
        else:
            self.surf.blit(surf, (x, y))

# ══════════════════════════════════════════════════════════════════════════════
#  PARTICLE SYSTEM
# ══════════════════════════════════════════════════════════════════════════════
class ParticleSystem:
    def __init__(self):
        self.particles = []

    def emit(self, pos, col, n=20, spd_range=(1,6), life_range=(20,50),
              size_range=(0.3,1.2), gravity=0.0):
        for _ in range(n):
            ang_h = random.uniform(0, 2*math.pi)
            ang_v = random.uniform(-math.pi/2, math.pi/2)
            spd   = random.uniform(*spd_range)
            vx = spd*math.cos(ang_v)*math.cos(ang_h)
            vy = spd*math.sin(ang_v)
            vz = spd*math.cos(ang_v)*math.sin(ang_h)
            self.particles.append({
                "pos":  np.array(pos, dtype=np.float32),
                "vel":  np.array([vx,vy,vz], dtype=np.float32),
                "col":  col,
                "life": random.randint(*life_range),
                "max_life": life_range[1],
                "size": random.uniform(*size_range),
                "gravity": gravity,
            })

    def emit_trail(self, pos, col, dir_vec, n=3):
        for _ in range(n):
            v = dir_vec * random.uniform(0.5,2.0) + \
                np.array([random.gauss(0,0.5), random.gauss(0,0.5), 0])
            self.particles.append({
                "pos":  np.array(pos, dtype=np.float32).copy(),
                "vel":  v.astype(np.float32),
                "col":  col,
                "life": random.randint(8, 18),
                "max_life": 18,
                "size": random.uniform(0.2, 0.6),
                "gravity": 0,
            })

    def update(self):
        alive = []
        for p in self.particles:
            p["pos"] += p["vel"]
            p["vel"][1] -= p["gravity"]
            p["life"]   -= 1
            if p["life"] > 0:
                alive.append(p)
        self.particles = alive

    def draw(self, rdr):
        if not rdr.use_gl: return
        glDisable(GL_DEPTH_TEST)
        for p in self.particles:
            alpha = p["life"] / p["max_life"]
            rdr.draw_particle_gl(p["pos"], p["size"]*alpha*2, p["col"], alpha)
        glEnable(GL_DEPTH_TEST)

# ══════════════════════════════════════════════════════════════════════════════
#  STARFIELD (3D tunnel effect)
# ══════════════════════════════════════════════════════════════════════════════
class Starfield:
    def __init__(self, n=800):
        self.stars = []
        for _ in range(n):
            self.stars.append(self._new_star(random.uniform(-FAR, 0)))

    def _new_star(self, z=None):
        if z is None: z = -FAR
        r = random.uniform(5, 200)
        a = random.uniform(0, 2*math.pi)
        return {
            "x": r*math.cos(a), "y": r*math.sin(a),
            "z": z,
            "size": random.uniform(0.3, 1.5),
            "col": random.choice([C_WHITE, C_CYAN, (0.7,0.7,1.0), (1.0,0.9,0.8)]),
            "speed": random.uniform(0.8, 3.0),
        }

    def update(self, speed_boost=1.0):
        for s in self.stars:
            s["z"] += s["speed"] * speed_boost
            if s["z"] > 10:
                s.update(self._new_star())

    def draw(self, rdr):
        if not rdr.use_gl: return
        glPointSize(2)
        glBegin(GL_POINTS)
        for s in self.stars:
            if s["z"] < -0.5:
                alpha = clamp(1 - (s["z"]+FAR)/FAR * 0.5, 0.1, 1.0)
                glColor4f(*s["col"], alpha)
                glVertex3f(s["x"], s["y"], s["z"])
        glEnd()

# ══════════════════════════════════════════════════════════════════════════════
#  ASTEROID
# ══════════════════════════════════════════════════════════════════════════════
class Asteroid:
    def __init__(self, cfg):
        self.pos   = v3(random.uniform(-60,60), random.uniform(-40,40), random.uniform(-300,-80))
        self.vel   = v3(random.uniform(-0.3,0.3), random.uniform(-0.2,0.2), random.uniform(0.8,2.5))
        self.size  = random.uniform(3, 10) + cfg["asteroid_density"]
        self.rot_y = random.uniform(0,360)
        self.rot_x = random.uniform(0,360)
        self.rot_spd = random.uniform(0.5,2.5)
        self.hp    = int(self.size * 1.5)
        self.col   = random.choice([(0.5,0.4,0.3),(0.6,0.5,0.4),(0.4,0.5,0.4)])
        self.alive = True

    def update(self):
        self.pos   += self.vel
        self.rot_y = (self.rot_y + self.rot_spd) % 360
        self.rot_x = (self.rot_x + self.rot_spd*0.7) % 360
        if self.pos[2] > 20:
            self.alive = False

    def draw(self, rdr):
        rdr.wire_box(self.pos, self.size*2, (0.6,0.5,0.4), 0.8,
                     rot_y=self.rot_y, rot_x=self.rot_x)

    def radius(self): return self.size

# ══════════════════════════════════════════════════════════════════════════════
#  BULLET
# ══════════════════════════════════════════════════════════════════════════════
class Bullet:
    def __init__(self, pos, vel, col, damage=1, is_enemy=False):
        self.pos      = np.array(pos, dtype=np.float32)
        self.vel      = np.array(vel, dtype=np.float32)
        self.col      = col
        self.damage   = damage
        self.is_enemy = is_enemy
        self.alive    = True
        self.age      = 0

    def update(self):
        self.pos += self.vel
        self.age += 1
        if self.age > 120 or self.pos[2] < -600 or self.pos[2] > 30:
            self.alive = False

    def draw(self, rdr):
        tail = self.pos - self.vel * 3
        rdr.draw_line(self.pos, tail, self.col, 1.0, 2.0)

    def radius(self): return 0.8

# ══════════════════════════════════════════════════════════════════════════════
#  MISSILE (homing)
# ══════════════════════════════════════════════════════════════════════════════
class Missile:
    def __init__(self, pos, target):
        self.pos    = np.array(pos, dtype=np.float32)
        self.target = target
        self.vel    = v3(0, 0, -18)
        self.alive  = True
        self.age    = 0
        self.col    = C_YELLOW
        self.damage = 5

    def update(self):
        self.age += 1
        if self.target and hasattr(self.target, 'alive') and self.target.alive:
            to_t = v3norm(self.target.pos - self.pos)
            self.vel = lerp(self.vel, to_t * 15, 0.06)
        self.pos += self.vel
        if self.age > 180 or self.pos[2] < -800:
            self.alive = False

    def draw(self, rdr):
        tail = self.pos + v3(0,0,2)
        rdr.draw_line(self.pos, tail, C_YELLOW, 1.0, 2.5)
        rdr.draw_sphere(self.pos, 0.6, C_ORANGE, 0.9, 6)

    def radius(self): return 1.2

# ══════════════════════════════════════════════════════════════════════════════
#  POWER-UP
# ══════════════════════════════════════════════════════════════════════════════
POWERUP_TYPES = [
    ("RAPID",   C_YELLOW, "Rapid Fire x2"),
    ("SPREAD",  C_GREEN,  "Spread Shot"),
    ("SHIELD",  C_CYAN,   "+1 Shield"),
    ("MISSILE", C_ORANGE, "+3 Missiles"),
    ("MULTI",   C_PINK,   "Triple Shot"),
    ("HEALTH",  C_GREEN,  "+2 HP"),
    ("BOMB",    C_RED,    "NOVA BOMB"),
    ("LASER",   C_PURPLE, "Laser Beam"),
    ("MAGNET",  C_BLUE,   "Score Magnet"),
    ("SLOW",    C_WHITE,  "Time Slow"),
]

class PowerUp:
    def __init__(self, pos):
        self.pos   = np.array(pos, dtype=np.float32)
        t          = random.choice(POWERUP_TYPES)
        self.kind, self.col, self.label = t
        self.vel   = v3(0, 0, 1.2)
        self.rot   = 0
        self.alive = True
        self.age   = 0

    def update(self):
        self.pos += self.vel
        self.rot  = (self.rot + 2) % 360
        self.age += 1
        if self.pos[2] > 20 or self.age > 600:
            self.alive = False

    def draw(self, rdr):
        rdr.wire_box(self.pos, 2.5, self.col, 0.9 + 0.1*math.sin(self.age*0.15),
                     rot_y=self.rot)
        rdr.draw_sphere(self.pos, 1.8, self.col, 0.3, 6)

    def radius(self): return 2.2

# ══════════════════════════════════════════════════════════════════════════════
#  ENEMY
# ══════════════════════════════════════════════════════════════════════════════
class Enemy:
    SHAPES = ["box","sphere","torus","diamond"]

    def __init__(self, pos, cfg, tier=1, formation_offset=None):
        ecfg        = ENEMY_CONFIGS[clamp(tier-1, 0, len(ENEMY_CONFIGS)-1)]
        self.tier   = ecfg[0]
        self.col    = ecfg[1]
        self.pos    = np.array(pos, dtype=np.float32)
        self.base_x = pos[0]
        self.base_y = pos[1]
        self.hp     = int(ecfg[2] * cfg["hp_mult"])
        self.max_hp = self.hp
        self.speed  = ecfg[5]*0.001 * cfg["speed_mult"]   # lateral wave speed
        self.fwd_speed = clamp(0.4 + tier*0.15, 0.4, 2.2) * cfg["speed_mult"]
        self.score  = int(ecfg[4] * (1 + cfg["level"]*0.02))
        self.shoot_cd = ecfg[5]
        self.shoot_t  = random.randint(30, ecfg[5])
        self.alive  = True
        self.age    = 0
        self.rot_y  = 0
        self.size   = 3 + tier * 1.2
        self.shape  = self.SHAPES[tier % len(self.SHAPES)]
        self.formation_off = formation_offset or v3()
        self.hit_flash = 0

    def update(self):
        self.age   += 1
        self.rot_y  = (self.rot_y + 1.5) % 360
        # wave motion
        self.pos[0] = self.base_x + math.sin(self.age * self.speed * 2) * 12
        self.pos[1] = self.base_y + math.cos(self.age * self.speed) * 6
        self.pos[2]+= self.fwd_speed
        if self.pos[2] > 25:
            self.alive = False
        if self.hit_flash > 0:
            self.hit_flash -= 1

    def can_shoot(self):
        self.shoot_t -= 1
        if self.shoot_t <= 0:
            self.shoot_t = self.shoot_cd + random.randint(-20,20)
            return True
        return False

    def shoot_bullet(self, player_pos):
        to_p = v3norm(player_pos - self.pos) * 8
        return Bullet(self.pos.copy(), to_p, self.col, damage=1, is_enemy=True)

    def draw(self, rdr):
        col = C_WHITE if self.hit_flash > 0 else self.col
        alpha = 1.0
        if self.shape == "box":
            rdr.wire_box(self.pos, self.size*2, col, alpha, rot_y=self.rot_y)
            rdr.solid_box(self.pos, self.size*1.6, (*col, 0.25), 0.25, rot_y=self.rot_y)
        elif self.shape == "sphere":
            rdr.draw_sphere(self.pos, self.size, col, alpha, 10)
        elif self.shape == "torus":
            rdr.draw_torus(self.pos, self.size*1.2, self.size*0.4, col, alpha, self.rot_y)
        elif self.shape == "diamond":
            rdr.wire_box(self.pos, self.size*2, col, alpha, rot_y=self.rot_y, rot_x=45)
        # HP bar (2D overlay done in HUD)
        # glow core
        rdr.draw_sphere(self.pos, self.size*0.4, col, 0.5, 4)

    def radius(self): return self.size

# ══════════════════════════════════════════════════════════════════════════════
#  BOSS
# ══════════════════════════════════════════════════════════════════════════════
class Boss:
    PHASES = 4

    def __init__(self, cfg):
        bc       = BOSS_CONFIGS[cfg["boss_idx"]]
        self.col = bc[0]
        self.hp  = int(bc[1] * 10 * (1 + cfg["level"]*0.01))
        self.max_hp = self.hp
        self.score  = int(bc[3] * (1 + cfg["level"]*0.05))
        self.pos    = v3(0, 0, -200)
        self.target_z = -60
        self.vel    = v3(0, 0, 0.8)
        self.phase  = 0
        self.age    = 0
        self.rot_y  = 0
        self.alive  = True
        self.size   = 14 + bc[2]*3
        self.shoot_cd = 40
        self.shoot_t  = 60
        self.pattern_t = 0
        self.hit_flash = 0
        self.minions   = []
        self.arrived   = False
        self.enrage    = False

    def update(self):
        self.age   += 1
        self.rot_y  = (self.rot_y + 0.8) % 360
        self.hit_flash = max(0, self.hit_flash-1)

        if not self.arrived:
            self.pos[2] = lerp(self.pos[2], self.target_z, 0.02)
            if abs(self.pos[2] - self.target_z) < 1:
                self.arrived = True
        else:
            # boss movement pattern
            t = self.age * 0.01
            self.pos[0] = math.sin(t * (1 + self.phase*0.3)) * (25 + self.phase*5)
            self.pos[1] = math.cos(t * 0.7) * 12

        # phase transitions
        hp_frac = self.hp / self.max_hp
        if hp_frac < 0.75 and self.phase < 1: self.phase = 1; sfx("boss")
        if hp_frac < 0.50 and self.phase < 2: self.phase = 2; sfx("boss")
        if hp_frac < 0.25 and self.phase < 3:
            self.phase = 3; self.enrage = True; sfx("boss")

    def can_shoot(self):
        self.shoot_t -= 1
        cd = self.shoot_cd // (1 + self.phase)
        if self.enrage: cd //= 2
        if self.shoot_t <= 0:
            self.shoot_t = cd
            return True
        return False

    def shoot_bullets(self, player_pos):
        bullets = []
        n_shots = 1 + self.phase
        if self.enrage: n_shots += 2
        for i in range(n_shots):
            angle = 2*math.pi*i/n_shots
            to_p = v3norm(player_pos - self.pos) * 9
            spread = v3(math.cos(angle)*2, math.sin(angle)*2, 0)
            vel = to_p + spread
            bullets.append(Bullet(self.pos.copy(), vel, self.col, damage=2, is_enemy=True))
        # radial burst in phase 2+
        if self.phase >= 2:
            for i in range(8):
                a = 2*math.pi*i/8 + self.age*0.05
                vel = v3(math.cos(a)*6, math.sin(a)*6, 3)
                bullets.append(Bullet(self.pos.copy(), vel, self.col, damage=1, is_enemy=True))
        return bullets

    def draw(self, rdr):
        col = C_WHITE if self.hit_flash > 0 else self.col
        # main body
        rdr.draw_torus(self.pos, self.size*0.8, self.size*0.25, col, 1.0, self.rot_y)
        rdr.wire_box(self.pos, self.size*1.4, col, 0.9, rot_y=self.rot_y)
        rdr.draw_sphere(self.pos, self.size*0.5, col, 0.6, 12)
        # satellite orbs
        n_orbs = 2 + self.phase
        for i in range(n_orbs):
            a = 2*math.pi*i/n_orbs + self.rot_y*0.04
            op = self.pos + v3(math.cos(a)*self.size*1.2, math.sin(a)*self.size*0.6, 0)
            rdr.draw_sphere(op, self.size*0.25, col, 0.8, 6)
        # enrage corona
        if self.enrage:
            rdr.draw_torus(self.pos, self.size*1.1, 0.8, C_RED, 0.6, -self.rot_y*1.5)

    def radius(self): return self.size * 0.9

# ══════════════════════════════════════════════════════════════════════════════
#  PLAYER SHIP
# ══════════════════════════════════════════════════════════════════════════════
class Player:
    def __init__(self):
        self.pos       = v3(0, 0, 0)
        self.vel       = v3()
        self.roll      = 0
        self.hp        = 5
        self.max_hp    = 5
        self.shield    = 2
        self.max_shield= 5
        self.missiles  = 3
        self.score     = 0
        self.lives     = 3
        self.xp        = 0
        self.level_xp  = 100

        # power-up states
        self.rapid_t   = 0
        self.spread    = False
        self.spread_t  = 0
        self.multi     = False
        self.multi_t   = 0
        self.laser     = False
        self.laser_t   = 0
        self.slow_t    = 0
        self.magnet_t  = 0

        self.shoot_cd  = 0
        self.shoot_delay = 14
        self.invincible= 0
        self.age       = 0
        self.alive     = True
        self.engine_flicker = 0

    def update(self, keys, dt=1.0):
        self.age += 1
        speed = 0.4

        # movement
        target = v3()
        if keys[pygame.K_LEFT]  or keys[pygame.K_a]: target[0] -= 1
        if keys[pygame.K_RIGHT] or keys[pygame.K_d]: target[0] += 1
        if keys[pygame.K_UP]    or keys[pygame.K_w]: target[1] += 1
        if keys[pygame.K_DOWN]  or keys[pygame.K_s]: target[1] -= 1

        # roll
        if keys[pygame.K_q]: self.roll = lerp(self.roll, -25, 0.1)
        elif keys[pygame.K_e]: self.roll = lerp(self.roll,  25, 0.1)
        else:                  self.roll = lerp(self.roll,    0, 0.12)

        self.vel = lerp(self.vel, target * speed * 7, 0.12)
        self.pos += self.vel
        self.pos[0] = clamp(self.pos[0], -45, 45)
        self.pos[1] = clamp(self.pos[1], -30, 30)
        self.pos[2] = 0

        # timers
        if self.shoot_cd > 0:   self.shoot_cd -= 1
        if self.invincible > 0: self.invincible -= 1
        if self.rapid_t > 0:
            self.rapid_t -= 1
        if self.spread_t > 0:
            self.spread_t -= 1
            if self.spread_t == 0: self.spread = False
        if self.multi_t > 0:
            self.multi_t -= 1
            if self.multi_t == 0: self.multi = False
        if self.laser_t > 0:
            self.laser_t -= 1
            if self.laser_t == 0: self.laser = False
        if self.slow_t > 0:    self.slow_t -= 1
        if self.magnet_t > 0:  self.magnet_t -= 1

        self.engine_flicker = random.randint(6,14)

    def shoot(self, ps):
        if self.shoot_cd > 0:
            return []
        delay = 6 if self.rapid_t > 0 else self.shoot_delay
        self.shoot_cd = delay
        sfx("shoot")
        bullets = []
        base = self.pos.copy()

        def add(off_x=0, vx=0):
            p = base + v3(off_x, 0, 0)
            v = v3(vx, 0, -20)
            bullets.append(Bullet(p, v, C_CYAN, damage=2))
            ps.emit_trail(p, C_CYAN, v3(0,0,1), n=2)

        add()
        if self.spread:
            add(-2, -3); add(2, 3)
        if self.multi:
            add(-3.5); add(3.5)

        return bullets

    def shoot_missile(self, target):
        if self.missiles <= 0: return None
        self.missiles -= 1
        sfx("missile")
        return Missile(self.pos.copy(), target)

    def hit(self, damage=1):
        if self.invincible > 0: return False
        if self.shield > 0:
            self.shield = max(0, self.shield-1)
            self.invincible = 50
            sfx("shield")
            return False
        self.hp -= damage
        self.invincible = 80
        sfx("hit")
        return self.hp <= 0

    def apply_powerup(self, kind):
        sfx("power")
        if kind == "RAPID":    self.rapid_t  = 360; return False
        if kind == "SPREAD":   self.spread = True; self.spread_t = 400; return False
        if kind == "SHIELD":   self.shield = min(self.shield+1, self.max_shield); return False
        if kind == "MISSILE":  self.missiles = min(self.missiles+3, 9); return False
        if kind == "MULTI":    self.multi = True; self.multi_t = 350; return False
        if kind == "HEALTH":   self.hp = min(self.hp+2, self.max_hp); return False
        if kind == "BOMB":     return True   # caller handles
        if kind == "LASER":    self.laser = True; self.laser_t = 300; return False
        if kind == "MAGNET":   self.magnet_t = 400; return False
        if kind == "SLOW":     self.slow_t = 300; return False
        return False

    def draw(self, rdr):
        if self.invincible > 0 and (self.invincible//5) % 2:
            return
        p = self.pos
        # fuselage
        rdr.wire_box(p, 4.0, C_CYAN, 1.0, rot_x=self.roll)
        # wings
        lw = p + v3(-5, 0, 1)
        rw = p + v3( 5, 0, 1)
        rdr.wire_box(lw, 3.5, C_BLUE, 0.8, rot_x=self.roll)
        rdr.wire_box(rw, 3.5, C_BLUE, 0.8, rot_x=self.roll)
        # cockpit glow
        rdr.draw_sphere(p + v3(0, 0.5, -1), 1.0, C_CYAN, 0.6, 5)
        # engines
        ef = self.engine_flicker * 0.07
        for ex in [-2.5, 2.5]:
            ep = p + v3(ex, -0.5, 2)
            rdr.draw_sphere(ep, ef, C_ORANGE, 0.9, 4)
        # shield ring
        if self.shield > 0:
            rdr.draw_sphere(p, 4.5 + self.shield*0.3,
                            C_CYAN, 0.15 + 0.05*self.shield, 8)
        # laser beam
        if self.laser:
            far = p + v3(0, 0, -600)
            rdr.draw_line(p, far, C_PURPLE, 0.7 + 0.3*math.sin(self.age*0.3), 3.0)

    def radius(self): return 2.5

# ══════════════════════════════════════════════════════════════════════════════
#  HUD  (2D overlay rendered via pygame fonts → OpenGL windowpos)
# ══════════════════════════════════════════════════════════════════════════════
FONT_BIG   = None
FONT_MED   = None
FONT_SM    = None
FONT_TINY  = None

def init_fonts():
    global FONT_BIG, FONT_MED, FONT_SM, FONT_TINY
    FONT_BIG  = pygame.font.SysFont("Consolas", 52, bold=True)
    FONT_MED  = pygame.font.SysFont("Consolas", 28, bold=True)
    FONT_SM   = pygame.font.SysFont("Consolas", 20, bold=True)
    FONT_TINY = pygame.font.SysFont("Consolas", 15)

def draw_hud(rdr, player, game):
    # HP
    rdr.blit_text(f"HP  {'█'*player.hp}{'░'*(player.max_hp-player.hp)}  {player.hp}/{player.max_hp}",
                  10, 10, (80,255,120), FONT_SM)
    # Shield
    rdr.blit_text(f"SHD {'◈'*player.shield}{'·'*(player.max_shield-player.shield)}",
                  10, 34, (0,210,255), FONT_SM)
    # Missiles
    rdr.blit_text(f"MSL {'▲'*player.missiles}",
                  10, 58, (255,200,0), FONT_SM)
    # Score
    sc = FONT_MED.render(f"◈ {player.score:>09,}", True, (255,230,0))
    if rdr.use_gl:
        data = pygame.image.tostring(sc, "RGBA", True)
        glWindowPos2i(W//2 - sc.get_width()//2, H - 10 - sc.get_height())
        glDrawPixels(sc.get_width(), sc.get_height(), GL_RGBA, GL_UNSIGNED_BYTE, data)
    # Level + Zone
    cfg  = game.level_cfg
    zone = cfg["zone"][0]
    rdr.blit_text(f"LV {game.current_level:>4}  WAVE {game.wave}/{cfg['n_waves']}  {zone}",
                  W - 380, 10, (160,160,200), FONT_TINY)
    # Power-up indicators
    px, py = 10, 90
    actives = []
    if player.rapid_t:   actives.append((f"RAPID {player.rapid_t//60+1}s",   (255,230,0)))
    if player.spread:    actives.append((f"SPREAD {player.spread_t//60+1}s",  (80,255,120)))
    if player.multi:     actives.append((f"TRIPLE {player.multi_t//60+1}s",   (255,80,180)))
    if player.laser:     actives.append((f"LASER {player.laser_t//60+1}s",    (180,0,255)))
    if player.slow_t:    actives.append((f"SLOW {player.slow_t//60+1}s",      (200,200,255)))
    if player.magnet_t:  actives.append((f"MAGNET {player.magnet_t//60+1}s",  (80,160,255)))
    for label, col in actives:
        rdr.blit_text(label, px, py, col, FONT_TINY); py += 18

    # Boss HP bar
    if game.boss and game.boss.alive:
        bw   = 500
        frac = game.boss.hp / game.boss.max_hp
        if rdr.use_gl:
            glDisable(GL_DEPTH_TEST)
            glBegin(GL_QUADS)
            glColor4f(0.3,0,0,0.8)
            glVertex2f(W//2-bw//2-2, H-50)
            glVertex2f(W//2+bw//2+2, H-50)
            glVertex2f(W//2+bw//2+2, H-26)
            glVertex2f(W//2-bw//2-2, H-26)
            glColor4f(1.0, 0.1+frac*0.5, 0, 0.9)
            glVertex2f(W//2-bw//2, H-48)
            glVertex2f(W//2-bw//2+int(bw*frac), H-48)
            glVertex2f(W//2-bw//2+int(bw*frac), H-28)
            glVertex2f(W//2-bw//2, H-28)
            glEnd()
            glEnable(GL_DEPTH_TEST)
        rdr.blit_text(f"BOSS  {game.boss.hp}/{game.boss.max_hp}",
                      W//2-60, H-60, (255,80,50), FONT_SM)

    # Lives
    rdr.blit_text(f"LIVES: {'♥'*player.lives}", W-150, H-28, (255,80,100), FONT_TINY)

# ══════════════════════════════════════════════════════════════════════════════
#  FORMATION SPAWNER
# ══════════════════════════════════════════════════════════════════════════════
def spawn_formation(cfg, ps_ref):
    enemies = []
    n       = cfg["enemies_per_wave"]
    form    = cfg["formation"]
    tier    = random.randint(1, cfg["tier_cap"])
    z_start = -250

    def mk(x, y):
        e = Enemy(v3(x, y, z_start), cfg, tier)
        enemies.append(e)

    if form == "scatter":
        for _ in range(n):
            mk(random.uniform(-40,40), random.uniform(-25,25))
    elif form == "v":
        for i in range(n):
            side = 1 if i%2==0 else -1
            mk(side*(i//2)*7, -(i//2)*5)
    elif form == "diamond":
        pts = [(0,0),(10,10),(-10,10),(20,0),(-20,0),(10,-10),(-10,-10),(0,-20)]
        for i in range(min(n, len(pts)*3)):
            px2, py2 = pts[i % len(pts)]
            mk(px2 + (i//len(pts))*3, py2 + (i//len(pts))*3)
    elif form == "ring":
        for i in range(n):
            a = 2*math.pi*i/n
            mk(math.cos(a)*25, math.sin(a)*16)
    elif form == "cross":
        for i in range(n):
            if i % 2 == 0:
                mk((i//2 - n//4)*7, 0)
            else:
                mk(0, (i//2 - n//4)*6)
    elif form == "spiral":
        for i in range(n):
            a = i * 0.6
            r = 3 + i*2
            mk(math.cos(a)*r, math.sin(a)*r*0.6)
    return enemies

# ══════════════════════════════════════════════════════════════════════════════
#  COLLISION
# ══════════════════════════════════════════════════════════════════════════════
def sphere_collide(a, b):
    return v3dist(a.pos, b.pos) < (a.radius() + b.radius())

# ══════════════════════════════════════════════════════════════════════════════
#  MAIN GAME  STATE MACHINE
# ══════════════════════════════════════════════════════════════════════════════
class Game:
    STATES = ["menu","play","pause","level_clear","gameover","victory"]

    def __init__(self):
        if OPENGL:
            self.screen = pygame.display.set_mode((W,H), pygame.OPENGL|pygame.DOUBLEBUF)
            glEnable(GL_BLEND)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
            glEnable(GL_DEPTH_TEST)
            glDepthFunc(GL_LEQUAL)
            glEnable(GL_LINE_SMOOTH)
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST)
        else:
            self.screen = pygame.display.set_mode((W,H))

        pygame.display.set_caption("GALACTIC FURY 3D  ·  900 LEVELS")
        self.rdr       = Renderer(self.screen)
        self.ps        = ParticleSystem()
        self.stars     = Starfield(1000)
        self.clock     = pygame.time.Clock()
        self.state     = "menu"
        self.high_score= 0
        self.total_kills = 0
        self.current_level = 1
        self.transition_timer = 0
        self.transition_msg   = ""
        init_fonts()
        self.new_game()

    def new_game(self):
        self.player    = Player()
        self.enemies   = []
        self.bullets   = []
        self.missiles  = []
        self.powerups  = []
        self.asteroids = []
        self.boss      = None
        self.wave      = 0
        self.wave_timer= 0
        self.wave_cd   = 200
        self.level_cfg = level_config(self.current_level)
        self.camera_shake = 0
        self.time_scale   = 1.0
        self.score_popup  = []   # [(text, x, y, life, col)]
        self.ps.particles.clear()
        self._spawn_asteroids()
        self.state     = "play"

    def _spawn_asteroids(self):
        n = self.level_cfg["asteroid_density"] * 4
        for _ in range(n):
            self.asteroids.append(Asteroid(self.level_cfg))

    # ── Main update ────────────────────────────────────────────────────────
    def update(self):
        if self.state != "play":
            for s in self.stars: s.update(0.3)
            return

        ts = self.time_scale
        if self.player.slow_t > 0:
            ts = 0.4

        keys = pygame.key.get_pressed()
        self.player.update(keys)

        # camera shake decay
        self.camera_shake = max(0, self.camera_shake - 0.4)

        # stars
        for s in self.stars: s.update(1.2 * ts)

        # wave management
        if not self.enemies and self.boss is None:
            self.wave_timer += 1
            if self.wave_timer >= self.wave_cd:
                self.wave_timer = 0
                self.wave += 1
                if self.wave > self.level_cfg["n_waves"]:
                    if self.level_cfg["boss"]:
                        if self.boss is None:
                            self.boss = Boss(self.level_cfg)
                            sfx("boss")
                    else:
                        self._level_complete()
                        return
                else:
                    self.enemies = spawn_formation(self.level_cfg, self.ps)

        # shoot
        if keys[pygame.K_SPACE]:
            bs = self.player.shoot(self.ps)
            self.bullets.extend(bs)

        # missile
        if keys[pygame.K_LSHIFT]:
            target = self._nearest_enemy()
            if target:
                m = self.player.shoot_missile(target)
                if m: self.missiles.append(m)

        # laser collision (continuous)
        if self.player.laser:
            lp = self.player.pos
            for e in list(self.enemies):
                if abs(e.pos[0]-lp[0]) < 2 and abs(e.pos[1]-lp[1]) < 2:
                    self._damage_enemy(e, 0.3)

        # update entities
        for b in self.bullets:  b.update()
        for m in self.missiles: m.update()
        for pu in self.powerups: pu.update()
        for ast in self.asteroids: ast.update()
        for e in self.enemies:  e.update()
        if self.boss: self.boss.update()

        # magnet: pull powerups
        if self.player.magnet_t > 0:
            for pu in self.powerups:
                to_p = v3norm(self.player.pos - pu.pos)
                pu.pos += to_p * 2.5

        self.ps.update()

        # score popups
        dead_pop = []
        for sp in self.score_popup:
            sp[3] -= 1
            if sp[3] <= 0: dead_pop.append(sp)
        for sp in dead_pop: self.score_popup.remove(sp)

        # boss shooting
        if self.boss and self.boss.arrived and self.boss.alive:
            if self.boss.can_shoot():
                bs = self.boss.shoot_bullets(self.player.pos)
                self.bullets.extend(bs)

        # boss spawning minions
        if self.boss and self.boss.age % 300 == 0 and self.boss.phase > 0:
            n_m = self.boss.phase
            for i in range(n_m):
                a = 2*math.pi*i/n_m
                mp = self.boss.pos + v3(math.cos(a)*20, math.sin(a)*12, 5)
                self.enemies.append(Enemy(mp, self.level_cfg, 1))

        # ── COLLISIONS ─────────────────────────────────────────────────────
        # player bullets vs enemies
        for b in list(self.bullets):
            if b.is_enemy: continue
            for e in list(self.enemies):
                if sphere_collide(b, e):
                    self._damage_enemy(e, b.damage)
                    self.ps.emit(b.pos, C_ORANGE, 8, (1,4))
                    b.alive = False
                    break
            if self.boss and sphere_collide(b, self.boss):
                self.boss.hp -= b.damage
                self.boss.hit_flash = 6
                self.ps.emit(b.pos, self.boss.col, 6, (1,3))
                b.alive = False
                if self.boss.hp <= 0:
                    self._kill_boss()

        # missiles vs enemies/boss
        for m in list(self.missiles):
            hit = False
            for e in list(self.enemies):
                if sphere_collide(m, e):
                    self.ps.emit(m.pos, C_YELLOW, 25, (2,7))
                    self._damage_enemy(e, m.damage)
                    sfx("explode")
                    m.alive = False; hit = True; break
            if not hit and self.boss and sphere_collide(m, self.boss):
                self.boss.hp -= m.damage
                self.boss.hit_flash = 8
                self.ps.emit(m.pos, C_ORANGE, 30, (2,8))
                sfx("explode")
                m.alive = False
                if self.boss.hp <= 0: self._kill_boss()

        # enemy bullets vs player
        for b in list(self.bullets):
            if not b.is_enemy: continue
            if sphere_collide(b, self.player):
                b.alive = False
                if self.player.hit(b.damage):
                    self._player_die()
                else:
                    self.camera_shake = 8
                    self.ps.emit(self.player.pos, C_RED, 12, (1,4))

        # enemy body vs player
        for e in list(self.enemies):
            if sphere_collide(e, self.player):
                if self.player.hit(2):
                    self._player_die()
                else:
                    self.camera_shake = 12

        # asteroid vs player
        for ast in list(self.asteroids):
            if v3dist(ast.pos, self.player.pos) < ast.radius() + self.player.radius():
                if self.player.hit(1):
                    self._player_die()

        # asteroid vs bullets
        for b in list(self.bullets):
            if b.is_enemy: continue
            for ast in list(self.asteroids):
                if v3dist(b.pos, ast.pos) < ast.radius():
                    ast.hp -= b.damage
                    b.alive = False
                    if ast.hp <= 0:
                        self.ps.emit(ast.pos, (0.7,0.5,0.3), 20, (1,5))
                        ast.alive = False
                    break

        # powerup collect
        for pu in list(self.powerups):
            if sphere_collide(pu, self.player):
                bomb = self.player.apply_powerup(pu.kind)
                pu.alive = False
                self.score_popup.append([f"+{pu.label}", self.player.pos[0],
                                         self.player.pos[1], 80, pu.col])
                if bomb:
                    self._nova_bomb()

        # boss death → level complete
        if self.boss and not self.boss.alive:
            self._level_complete()

        # prune dead
        self.bullets   = [b for b in self.bullets  if b.alive]
        self.missiles  = [m for m in self.missiles if m.alive]
        self.powerups  = [p for p in self.powerups if p.alive]
        self.asteroids = [a for a in self.asteroids if a.alive]
        self.enemies   = [e for e in self.enemies  if e.alive]

        # respawn asteroids
        while len(self.asteroids) < self.level_cfg["asteroid_density"]*4:
            self.asteroids.append(Asteroid(self.level_cfg))

    def _damage_enemy(self, e, dmg):
        e.hp -= dmg
        e.hit_flash = 5
        if e.hp <= 0:
            self.ps.emit(e.pos, e.col, 22, (2,7))
            sfx("explode")
            self.player.score += e.score
            self.total_kills  += 1
            self.score_popup.append([f"+{e.score}", e.pos[0], e.pos[1], 60, e.col])
            if random.random() < self.level_cfg["powerup_chance"]:
                self.powerups.append(PowerUp(e.pos.copy()))
            e.alive = False

    def _kill_boss(self):
        if not self.boss: return
        self.ps.emit(self.boss.pos, self.boss.col, 80, (3,12))
        sfx("explode")
        self.player.score += self.boss.score
        self.total_kills  += 1
        self.score_popup.append([f"BOSS! +{self.boss.score}", 0, 0, 120, C_YELLOW])
        for _ in range(4):
            pp = self.boss.pos + v3(random.uniform(-10,10),random.uniform(-10,10),0)
            self.powerups.append(PowerUp(pp))
        self.boss.alive = False

    def _nova_bomb(self):
        for e in self.enemies:
            self.ps.emit(e.pos, e.col, 15, (2,6))
            self.player.score += e.score
        self.enemies.clear()
        if self.boss:
            self.boss.hp = max(1, self.boss.hp - self.boss.max_hp//4)
        sfx("explode")
        self.camera_shake = 20

    def _player_die(self):
        self.ps.emit(self.player.pos, C_CYAN, 40, (2,8))
        sfx("explode")
        self.camera_shake = 20
        self.player.lives -= 1
        if self.player.lives <= 0:
            if self.player.score > self.high_score:
                self.high_score = self.player.score
            self.state = "gameover"
        else:
            self.player.hp       = self.player.max_hp
            self.player.invincible = 180
            self.player.pos      = v3(0,0,0)

    def _nearest_enemy(self):
        targets = self.enemies + ([self.boss] if self.boss else [])
        if not targets: return None
        return min(targets, key=lambda e: v3dist(e.pos, self.player.pos))

    def _level_complete(self):
        sfx("level")
        self.player.score += (self.current_level * 100)
        if self.current_level >= MAX_LEVELS:
            self.state = "victory"
            return
        self.current_level += 1
        if self.current_level > MAX_LEVELS:
            self.state = "victory"; return
        self.state = "level_clear"
        self.transition_timer = 180

    # ── Draw ───────────────────────────────────────────────────────────────
    def draw(self):
        zone = self.level_cfg["zone"]
        bg   = zone[2:5]
        self.rdr.begin_frame(bg)

        if self.state in ("play","pause","level_clear"):
            if self.rdr.use_gl:
                glEnable(GL_DEPTH_TEST)
                self.rdr.set_3d()

                # camera
                sx = math.sin(self.camera_shake*0.3)*self.camera_shake*0.05
                sy = math.cos(self.camera_shake*0.4)*self.camera_shake*0.04
                glTranslatef(-self.player.pos[0]*0.08 + sx,
                             -self.player.pos[1]*0.08 + sy, 0)

                # fog
                glFogi(GL_FOG_MODE, GL_LINEAR)
                glFogf(GL_FOG_START, 200)
                glFogf(GL_FOG_END,   FAR*0.8)
                glFogfv(GL_FOG_COLOR, (*bg, 1.0))
                glEnable(GL_FOG)

                # stars
                self.stars.draw(self.rdr)

                # asteroids
                for ast in self.asteroids: ast.draw(self.rdr)

                # enemies
                for e in self.enemies: e.draw(self.rdr)

                # boss
                if self.boss and self.boss.alive: self.boss.draw(self.rdr)

                # bullets
                for b in self.bullets:  b.draw(self.rdr)
                for m in self.missiles: m.draw(self.rdr)

                # powerups
                for pu in self.powerups: pu.draw(self.rdr)

                # particles
                self.ps.draw(self.rdr)

                # player
                self.player.draw(self.rdr)

                glDisable(GL_FOG)

                # 2D overlay
                self.rdr.set_2d()
                draw_hud(self.rdr, self.player, self)

                # score popups (2D)
                for sp in self.score_popup:
                    alpha = sp[3] / 80
                    col3  = tuple(int(c*255*alpha) for c in sp[4])
                    px2   = int(W//2 + sp[0]*3)
                    py2   = int(H//2 - sp[1]*8 - (80-sp[3])*0.4)
                    self.rdr.blit_text(sp[0], px2, py2, col3, FONT_TINY)

            if self.state == "pause":
                self._draw_overlay("PAUSED", "[P] Resume   [ESC] Menu")
            elif self.state == "level_clear":
                self.transition_timer -= 1
                msg = f"LEVEL {self.current_level-1} CLEAR!"
                sub = f"Advancing to Level {self.current_level} · Zone: {self.level_cfg['zone'][0]}"
                self._draw_overlay(msg, sub, col=(255,230,0))
                if self.transition_timer <= 0:
                    self.new_game()

        elif self.state == "menu":
            self._draw_menu()
        elif self.state == "gameover":
            self._draw_gameover()
        elif self.state == "victory":
            self._draw_victory()

        pygame.display.flip()

    def _draw_overlay(self, title, sub, col=(255,255,255)):
        if self.rdr.use_gl:
            glDisable(GL_DEPTH_TEST)
            # dim background
            glColor4f(0,0,0,0.55)
            glBegin(GL_QUADS)
            glVertex2f(0,0); glVertex2f(W,0); glVertex2f(W,H); glVertex2f(0,H)
            glEnd()
        self.rdr.blit_text(title, W//2 - len(title)*16, H//2-60, col, FONT_BIG)
        self.rdr.blit_text(sub,   W//2 - len(sub)*7,    H//2+20, (200,200,200), FONT_SM)

    def _draw_menu(self):
        if self.rdr.use_gl:
            self.rdr.set_2d()
            glDisable(GL_DEPTH_TEST)
            glColor4f(0,0,0,1)
            glBegin(GL_QUADS)
            glVertex2f(0,0); glVertex2f(W,0); glVertex2f(W,H); glVertex2f(0,H)
            glEnd()
        lines = [
            ("GALACTIC FURY 3D", W//2-210, H//2-170, (0,220,255),   FONT_BIG),
            ("900 LEVELS  ·  10 ZONES  ·  BOSS ENCOUNTERS",
                                  W//2-260, H//2- 80, (160,160,220), FONT_SM),
            ("──────────────────────────────────────────",
                                  W//2-260, H//2- 50, (60,60,100),   FONT_TINY),
            ("WASD / Arrows  →  Move",     W//2-160, H//2- 20, (200,200,200), FONT_SM),
            ("Space          →  Shoot",    W//2-160, H//2+ 10, (200,200,200), FONT_SM),
            ("LShift         →  Missile",  W//2-160, H//2+ 40, (200,200,200), FONT_SM),
            ("Q / E          →  Roll",     W//2-160, H//2+ 70, (200,200,200), FONT_SM),
            ("P              →  Pause",    W//2-160, H//2+100, (200,200,200), FONT_SM),
            (f"HIGH SCORE:  {self.high_score:,}",
                                  W//2-130, H//2+150, (255,220,0),   FONT_MED),
            (f"TOTAL KILLS: {self.total_kills:,}",
                                  W//2-130, H//2+185, (100,255,150), FONT_SM),
            ("► PRESS  ENTER  TO LAUNCH ◄",W//2-185, H//2+240, (255,180,0), FONT_MED),
            (f"Starting at Level {self.current_level}",
                                  W//2-110, H//2+280, (120,120,180), FONT_TINY),
        ]
        for txt, x, y, col, fnt in lines:
            self.rdr.blit_text(txt, x, y, col, fnt)

    def _draw_gameover(self):
        if self.rdr.use_gl:
            self.rdr.set_2d()
            glColor4f(0,0,0,1)
            glBegin(GL_QUADS)
            glVertex2f(0,0); glVertex2f(W,0); glVertex2f(W,H); glVertex2f(0,H)
            glEnd()
        lines = [
            ("GAME OVER",               W//2-180, H//2-150, (255,50,50),  FONT_BIG),
            (f"Score:   {self.player.score:,}", W//2-130, H//2- 50, (255,230,0),  FONT_MED),
            (f"Best:    {self.high_score:,}",   W//2-130, H//2- 10, (100,255,150),FONT_MED),
            (f"Level:   {self.current_level}",  W//2-130, H//2+ 30, (0,210,255),  FONT_MED),
            (f"Kills:   {self.total_kills:,}",  W//2-130, H//2+ 70, (200,200,200),FONT_SM),
            ("[R] Retry   [M] Menu   [ESC] Quit",W//2-195,H//2+140,(255,180,0),   FONT_MED),
        ]
        for txt, x, y, col, fnt in lines:
            self.rdr.blit_text(txt, x, y, col, fnt)

    def _draw_victory(self):
        if self.rdr.use_gl:
            self.rdr.set_2d()
            glColor4f(0,0,0.05,1)
            glBegin(GL_QUADS)
            glVertex2f(0,0); glVertex2f(W,0); glVertex2f(W,H); glVertex2f(0,H)
            glEnd()
        t = time.time()
        r = int(127+127*math.sin(t*2))
        g = int(127+127*math.sin(t*2+2))
        b = int(127+127*math.sin(t*2+4))
        lines = [
            ("YOU WIN!",             W//2-160, H//2-160, (r,g,b),       FONT_BIG),
            ("ALL 900 LEVELS CONQUERED",W//2-220,H//2- 70,(255,230,0),  FONT_MED),
            (f"Final Score: {self.player.score:,}", W//2-160,H//2- 20,(255,255,255),FONT_MED),
            (f"Total Kills: {self.total_kills:,}",  W//2-150,H//2+ 30,(100,255,150),FONT_SM),
            ("You are a galactic legend.",           W//2-185,H//2+100,(200,200,255),FONT_SM),
            ("[R] Play Again   [ESC] Quit",          W//2-185,H//2+160,(255,180,0), FONT_MED),
        ]
        for txt, x, y, col, fnt in lines:
            self.rdr.blit_text(txt, x, y, col, fnt)

    # ── Event handler ──────────────────────────────────────────────────────
    def handle_event(self, event):
        if event.type == pygame.QUIT:
            return False
        if event.type == pygame.KEYDOWN:
            k = event.key
            if k == pygame.K_ESCAPE:
                if self.state == "play":
                    self.state = "pause"
                elif self.state == "pause":
                    self.state = "menu"
                    self.current_level = 1
                elif self.state in ("menu","gameover","victory"):
                    return False
            elif k == pygame.K_p:
                if self.state == "play":   self.state = "pause"
                elif self.state == "pause": self.state = "play"
            elif k == pygame.K_RETURN:
                if self.state in ("menu","pause"):
                    self.new_game()
                    self.state = "play"
            elif k == pygame.K_r:
                if self.state in ("gameover","victory"):
                    self.current_level = 1
                    self.player.score  = 0
                    self.total_kills   = 0
                    self.new_game()
                    self.state = "play"
            elif k == pygame.K_m:
                if self.state == "gameover":
                    self.state = "menu"
                    self.current_level = 1
        return True

    def run(self):
        running = True
        while running:
            self.clock.tick(FPS)
            for event in pygame.event.get():
                if not self.handle_event(event):
                    running = False
            keys = pygame.key.get_pressed()
            if self.state == "menu" and keys[pygame.K_RETURN]:
                self.new_game(); self.state = "play"
            self.update()
            self.draw()
        pygame.quit()
        sys.exit()

# ══════════════════════════════════════════════════════════════════════════════
#  ENTRY POINT
# ══════════════════════════════════════════════════════════════════════════════
if __name__ == "__main__":
    if not OPENGL:
        print("─"*55)
        print("  PyOpenGL not installed. Install with:")
        print("  pip install PyOpenGL PyOpenGL_accelerate numpy")
        print("─"*55)
        sys.exit(1)
    game = Game()
    game.run()
