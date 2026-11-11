#!/usr/bin/env python3
"""Preview the TESTING.md scenario recordings and mark the good ones "keep".

    python tools/testing_scenarios/review_videos.py

Lists tools/testing_scenarios/recordings/*.mp4 (and recordings/keep/, recordings/throwaway/) in TESTING.md order.
"Keep" (K) moves a video to recordings/keep/ (tracked by git); "Highlight" (H) to recordings/keep/highlights/ (tracked
too, apart from the other kept ones: the videos worth showing); "Throwaway" (T) to recordings/throwaway/ (ignored by
git): a video you are done with but do not want in the repository. run_tests.ps1 skips the scripts whose video is in
any of them (-IncludeKept runs them anyway). Pressing the same key again moves the video back. Thrown-away videos are
hidden unless "Show thrown away" is ticked.

Keys: F5 refresh the videos that changed on disk, Space play/pause, Left/Right one frame (Shift: 1 s), Home/End, Up/Down previous/next video, K keep, H highlight, T throwaway,
L loop, 1-4 zoom, [ ] speed.

Needs ffmpeg on PATH (the recordings already need it) and Pillow.
"""

import os
import re
import shutil
import subprocess
import sys
import threading
import time
import tkinter as tk
import zlib
from pathlib import Path
from tkinter import messagebox, ttk

try:
    from PIL import Image, ImageTk
except ImportError:                                     # (the message box needs a root window)
    root = tk.Tk()
    root.withdraw()
    messagebox.showerror("review_videos", "Pillow is missing: pip install pillow")
    sys.exit(1)

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
RECORDINGS = HERE / "recordings"
KEEP = RECORDINGS / "keep"
THROWAWAY = RECORDINGS / "throwaway"
HIGHLIGHTS = KEEP / "highlights"
# a video's status: where it lives (None: recordings/ itself, still to review)
FOLDERS = {None: RECORDINGS, "keep": KEEP, "highlight": HIGHLIGHTS, "throw": THROWAWAY}
MARKS = {"keep": "★ ", "highlight": "◆ ", "throw": "✗ "}
LABELS = {"keep": "   (kept)", "highlight": "   (highlight)", "throw": "   (thrown away)"}
COLOURS = {"keep": "#2e7d32", "highlight": "#b8860b", "throw": "#888888"}
TESTING_MD = REPO / "TESTING.md"

SRC_W, SRC_H = 240, 160                                 # (decoded at the GBA's size; zoomed with nearest neighbour)
SPEEDS = [0.25, 0.5, 1.0, 2.0, 4.0]
NO_WINDOW = 0x08000000 if os.name == "nt" else 0       # CREATE_NO_WINDOW: no console flashing up for ffmpeg


def testing_order():
    """Script names in TESTING.md order (each scenario's line starts with its name in backticks)."""
    order = {}
    if TESTING_MD.is_file():
        for line in TESTING_MD.read_text(encoding="utf-8").splitlines():
            m = re.match(r"^`([A-Za-z0-9_]+)`", line)
            if m and m.group(1) not in order:
                order[m.group(1)] = len(order)
    return order


FPS = 60.0                                              # (lib_eds.lua encodes every recording at 60 fps)
PREFETCH = 2                                            # videos after the current one decoded ahead (ffmpeg takes
                                                        # ~3 s to start here; the decode itself ~1 s)


def file_stamp(path):
    """The file's modification time and size, to see that it changed (None when it is gone)."""
    try:
        st = os.stat(str(path))
        return st.st_mtime_ns, st.st_size
    except OSError:
        return None


class Decoder:
    """Decodes a video into zlib-compressed RGB frames in a thread; frames become available as they arrive."""

    def __init__(self, path):
        self.path = path
        self.stamp = file_stamp(path)               # (Refresh videos: the file as it was decoded)
        self.frames = []
        self.done = False
        self.error = None
        self._stop = False
        self._proc = None
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _run(self):
        size = SRC_W * SRC_H * 3
        try:
            self._proc = subprocess.Popen(
                ["ffmpeg", "-v", "error", "-i", str(self.path), "-vf", "scale=%d:%d:flags=area" % (SRC_W, SRC_H),
                 "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, creationflags=NO_WINDOW)
            while not self._stop:
                buf = self._proc.stdout.read(size)
                if len(buf) < size:
                    break
                self.frames.append(zlib.compress(buf, 1))
        except Exception as e:                          # (ffmpeg missing, file gone)
            self.error = str(e)
        finally:
            self.close_process()
            self.done = True

    def close_process(self):
        p = self._proc
        if p and p.poll() is None:
            try:
                p.kill()
            except Exception:
                pass
        if p:
            try:
                p.stdout.close()
            except Exception:
                pass

    def stop(self):
        """Stops decoding and waits for ffmpeg to let go of the file (Windows will not move an open file)."""
        self._stop = True
        self.close_process()
        self._thread.join(timeout=3)

    def image(self, i):
        return Image.frombytes("RGB", (SRC_W, SRC_H), zlib.decompress(self.frames[i]))


class App:
    def __init__(self, root):
        self.root = root
        self.decoder = None
        self.cache = {}                                 # path -> Decoder: the current video and the next ones
        self.fps = FPS
        self.pos = 0
        self.playing = False
        self.loop = tk.BooleanVar(value=True)
        self.show_kept = tk.BooleanVar(value=True)
        self.show_thrown = tk.BooleanVar(value=False)
        self.show_highlights = tk.BooleanVar(value=True)
        self.filter = tk.StringVar()
        self.zoom = 3
        self.speed_i = SPEEDS.index(1.0)
        self.videos = []                                # [(name, path, status)] in the list's order
        self._photo = None
        self._last_tick = None
        self._slider_busy = False
        self._autoplay = False                          # (videos play on opening, except the first one)

        root.title("Test video review")
        root.geometry("1180x640")
        root.minsize(900, 520)

        left = ttk.Frame(root, padding=6)
        left.pack(side=tk.LEFT, fill=tk.Y)
        ttk.Label(left, text="Filter").pack(anchor="w")
        ent = ttk.Entry(left, textvariable=self.filter)
        ent.pack(fill=tk.X)
        self.filter.trace_add("write", lambda *a: self.refresh_list())
        ttk.Checkbutton(left, text="Show kept videos", variable=self.show_kept,
                        command=self.refresh_list).pack(anchor="w", pady=(4, 0))
        ttk.Checkbutton(left, text="Show highlights", variable=self.show_highlights,
                        command=self.refresh_list).pack(anchor="w")
        ttk.Checkbutton(left, text="Show thrown away", variable=self.show_thrown,
                        command=self.refresh_list).pack(anchor="w", pady=(0, 2))
        box = ttk.Frame(left)
        box.pack(fill=tk.Y, expand=True)
        self.listbox = tk.Listbox(box, width=52, activestyle="dotbox", exportselection=False,
                                  font=("Consolas", 9))
        sb = ttk.Scrollbar(box, orient=tk.VERTICAL, command=self.listbox.yview)
        self.listbox.configure(yscrollcommand=sb.set)
        self.listbox.pack(side=tk.LEFT, fill=tk.Y, expand=True)
        sb.pack(side=tk.LEFT, fill=tk.Y)
        self.listbox.bind("<<ListboxSelect>>", lambda e: self.open_selected())
        self.count_label = ttk.Label(left, text="")
        self.count_label.pack(anchor="w", pady=(4, 0))

        right = ttk.Frame(root, padding=6)
        right.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.title_label = ttk.Label(right, text="", font=("Segoe UI", 11, "bold"))
        self.title_label.pack(anchor="w")
        self.canvas = tk.Label(right, background="black")
        self.canvas.pack(fill=tk.BOTH, expand=True, pady=4)

        self.slider = ttk.Scale(right, from_=0, to=1, orient=tk.HORIZONTAL, command=self.on_slider)
        self.slider.pack(fill=tk.X)

        bar = ttk.Frame(right)
        bar.pack(fill=tk.X, pady=(4, 0))
        ttk.Button(bar, text="⏮", width=3, command=lambda: self.seek(0)).pack(side=tk.LEFT)
        ttk.Button(bar, text="◀", width=3, command=lambda: self.step(-1)).pack(side=tk.LEFT)
        self.play_btn = ttk.Button(bar, text="▶ Play", width=8, command=self.toggle_play)
        self.play_btn.pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="▶", width=3, command=lambda: self.step(1)).pack(side=tk.LEFT)
        ttk.Button(bar, text="⏭", width=3, command=lambda: self.seek(10 ** 9)).pack(side=tk.LEFT)
        ttk.Checkbutton(bar, text="Loop", variable=self.loop).pack(side=tk.LEFT, padx=8)
        ttk.Label(bar, text="Speed").pack(side=tk.LEFT)
        self.speed_box = ttk.Combobox(bar, width=6, state="readonly", values=["%gx" % s for s in SPEEDS])
        self.speed_box.current(self.speed_i)
        self.speed_box.bind("<<ComboboxSelected>>", lambda e: self.set_speed(self.speed_box.current()))
        self.speed_box.pack(side=tk.LEFT, padx=(2, 8))
        ttk.Label(bar, text="Zoom").pack(side=tk.LEFT)
        self.zoom_box = ttk.Combobox(bar, width=4, state="readonly", values=["1x", "2x", "3x", "4x"])
        self.zoom_box.current(self.zoom - 1)
        self.zoom_box.bind("<<ComboboxSelected>>", lambda e: self.set_zoom(self.zoom_box.current() + 1))
        self.zoom_box.pack(side=tk.LEFT, padx=2)
        self.time_label = ttk.Label(bar, text="", width=26)
        self.time_label.pack(side=tk.LEFT, padx=8)

        bar2 = ttk.Frame(right)
        bar2.pack(fill=tk.X, pady=(6, 0))
        ttk.Button(bar2, text="↑ Previous", command=lambda: self.move_selection(-1)).pack(side=tk.LEFT)
        ttk.Button(bar2, text="↓ Next", command=lambda: self.move_selection(1)).pack(side=tk.LEFT, padx=4)
        self.keep_btn = ttk.Button(bar2, text="★ Keep (K)", command=self.toggle_keep)
        self.keep_btn.pack(side=tk.LEFT, padx=(12, 4))
        self.highlight_btn = ttk.Button(bar2, text="◆ Highlight (H)", command=self.toggle_highlight)
        self.highlight_btn.pack(side=tk.LEFT, padx=(0, 4))
        self.throw_btn = ttk.Button(bar2, text="✗ Throwaway (T)", command=self.toggle_throw)
        self.throw_btn.pack(side=tk.LEFT, padx=(0, 12))
        ttk.Button(bar2, text="Open folder", command=self.open_folder).pack(side=tk.LEFT)
        ttk.Button(bar2, text="Rescan", command=self.refresh_list).pack(side=tk.LEFT, padx=4)
        ttk.Button(bar2, text="⟳ Refresh videos (F5)", command=self.refresh_videos).pack(side=tk.LEFT)
        self.copy_btn = ttk.Button(bar2, text="⧉ Copy title (C)", command=self.copy_title)
        self.copy_btn.pack(side=tk.LEFT, padx=4)
        ttk.Label(bar2, text="Space play · ←/→ frame (Shift 1 s) · ↑/↓ video · K keep · H highlight · T throwaway · C copy title · L loop · [ ] speed",
                  foreground="#666").pack(side=tk.RIGHT)

        for key, fn in [("<space>", lambda e: self.toggle_play()),
                        ("<Left>", lambda e: self.step(-1)), ("<Right>", lambda e: self.step(1)),
                        ("<Shift-Left>", lambda e: self.step(-int(self.fps))),
                        ("<Shift-Right>", lambda e: self.step(int(self.fps))),
                        ("<Home>", lambda e: self.seek(0)), ("<End>", lambda e: self.seek(10 ** 9)),
                        ("<Up>", lambda e: self.move_selection(-1)), ("<Down>", lambda e: self.move_selection(1)),
                        ("<k>", lambda e: self.toggle_keep()), ("<K>", lambda e: self.toggle_keep()),
                        ("<t>", lambda e: self.toggle_throw()), ("<T>", lambda e: self.toggle_throw()),
                        ("<F5>", lambda e: self.refresh_videos()),
                        ("<c>", lambda e: self.copy_title()), ("<C>", lambda e: self.copy_title()),
                        ("<h>", lambda e: self.toggle_highlight()), ("<H>", lambda e: self.toggle_highlight()),
                        ("<l>", lambda e: self.loop.set(not self.loop.get())),
                        ("<bracketleft>", lambda e: self.set_speed(self.speed_i - 1)),
                        ("<bracketright>", lambda e: self.set_speed(self.speed_i + 1))]:
            root.bind(key, self._unless_typing(fn))
        for n in range(1, 5):
            root.bind(str(n), self._unless_typing(lambda e, n=n: self.set_zoom(n)))
        # (the list's own arrow keys would move twice)
        self.listbox.bind("<Up>", lambda e: (self.move_selection(-1), "break")[1])
        self.listbox.bind("<Down>", lambda e: (self.move_selection(1), "break")[1])
        root.protocol("WM_DELETE_WINDOW", self.quit)

        self.refresh_list()
        if self.videos:
            self.select(0)
        self.tick()

    # ---- the list ----

    def _unless_typing(self, fn):
        def handler(e):
            if isinstance(e.widget, (tk.Entry, ttk.Entry)):
                return None
            fn(e)
            return "break"
        return handler

    def scan(self):
        order = testing_order()
        found = {}
        for status, folder in FOLDERS.items():         # (a name in keep/ or throwaway/ wins over recordings/)
            if folder.is_dir():
                for path in folder.glob("*.mp4"):
                    if status or path.stem not in found:
                        found[path.stem] = (path, status)
        names = sorted(found, key=lambda n: (order.get(n, 10 ** 9), n))
        return [(n, found[n][0], found[n][1]) for n in names]

    def refresh_list(self, keep_name=None):
        current = keep_name or self.current_name()
        allv = self.scan()
        f = self.filter.get().strip().lower()
        shown = {None: True, "keep": self.show_kept.get(), "highlight": self.show_highlights.get(),
                 "throw": self.show_thrown.get()}
        self.videos = [v for v in allv if shown[v[2]] and (not f or f in v[0].lower())]
        self.listbox.delete(0, tk.END)
        for name, _, status in self.videos:
            self.listbox.insert(tk.END, MARKS.get(status, "   ") + name)
            if status:
                self.listbox.itemconfig(tk.END, foreground=COLOURS[status])
        n = {s: sum(1 for v in allv if v[2] == s) for s in MARKS}
        self.count_label.configure(text="%d videos · %d kept · %d highlights · %d thrown away · %d to review" % (
            len(allv), n["keep"], n["highlight"], n["throw"], len(allv) - sum(n.values())))
        for i, v in enumerate(self.videos):
            if v[0] == current:
                self.listbox.selection_clear(0, tk.END)
                self.listbox.selection_set(i)
                self.listbox.see(i)
                break

    def current_name(self):
        sel = self.listbox.curselection() if hasattr(self, "listbox") else ()
        return self.videos[sel[0]][0] if sel and sel[0] < len(self.videos) else None

    def select(self, i):
        if not self.videos:
            return
        i = max(0, min(i, len(self.videos) - 1))
        self.listbox.selection_clear(0, tk.END)
        self.listbox.selection_set(i)
        self.listbox.see(i)
        self.open_selected()

    def move_selection(self, d):
        sel = self.listbox.curselection()
        self.select((sel[0] + d) if sel else 0)

    # ---- playback ----

    def open_selected(self):
        sel = self.listbox.curselection()
        if not sel:
            return
        name, path, status = self.videos[sel[0]]
        if self.decoder and self.decoder.path == path:
            return
        self.close_video()
        self.decoder = self.decoder_for(path)
        self.prefetch(sel[0])
        self.pos = 0
        self.playing = self._autoplay           # (the first video at startup waits, paused on its first frame)
        self._autoplay = True
        self._last_tick = None
        self.title_label.configure(text=MARKS.get(status, "") + name + LABELS.get(status, ""))
        self.keep_btn.configure(text="Unkeep (K)" if status == "keep" else "★ Keep (K)")
        self.highlight_btn.configure(text="Unhighlight (H)" if status == "highlight" else "◆ Highlight (H)")
        self.throw_btn.configure(text="Restore (T)" if status == "throw" else "✗ Throwaway (T)")
        self.play_btn.configure(text="⏸ Pause" if self.playing else "▶ Play")

    def decoder_for(self, path):
        d = self.cache.get(path)
        if d is None or d.error:
            d = self.cache[path] = Decoder(path)
        return d

    def prefetch(self, i):
        """Decodes the next PREFETCH videos ahead; drops the decoders of the others (their memory)."""
        wanted = {self.videos[j][1] for j in range(i, min(i + 1 + PREFETCH, len(self.videos)))}
        if i > 0:
            wanted.add(self.videos[i - 1][1])          # (and the previous one, for going back)
        for path in list(self.cache):
            if path not in wanted:
                self.cache.pop(path).stop()
        for path in wanted:
            self.decoder_for(path)

    def release(self, path):
        """Stops the decoder of `path` (before moving the file: Windows will not move an open file)."""
        d = self.cache.pop(path, None)
        if d:
            d.stop()
        if self.decoder is d:
            self.decoder = None

    def close_video(self):
        self.decoder = None                             # (its decoder stays in the cache)
        self._photo = None
        self.canvas.configure(image="")

    def toggle_play(self):
        if not self.decoder:
            return
        if not self.playing and self.decoder.done and self.pos >= len(self.decoder.frames) - 1:
            self.pos = 0
        self.playing = not self.playing
        self._last_tick = None
        self.play_btn.configure(text="⏸ Pause" if self.playing else "▶ Play")

    def step(self, d):
        if self.decoder:
            self.playing = False
            self.play_btn.configure(text="▶ Play")
            self.seek(self.pos + d)

    def seek(self, i):
        if not self.decoder or not self.decoder.frames:
            return
        self.pos = max(0, min(int(i), len(self.decoder.frames) - 1))
        self.show()

    def on_slider(self, value):
        if self._slider_busy or not self.decoder:
            return
        self.seek(float(value))

    def set_speed(self, i):
        self.speed_i = max(0, min(i, len(SPEEDS) - 1))
        self.speed_box.current(self.speed_i)

    def set_zoom(self, z):
        self.zoom = z
        self.zoom_box.current(z - 1)
        self.show()

    def show(self):
        d = self.decoder
        if not d or not d.frames:
            return
        self.pos = min(self.pos, len(d.frames) - 1)
        img = d.image(self.pos)
        if self.zoom != 1:
            img = img.resize((SRC_W * self.zoom, SRC_H * self.zoom), Image.NEAREST)
        self._photo = ImageTk.PhotoImage(img)
        self.canvas.configure(image=self._photo)
        total = len(d.frames)
        self._slider_busy = True
        self.slider.configure(to=max(1, total - 1))
        self.slider.set(self.pos)
        self._slider_busy = False
        self.time_label.configure(text="frame %d / %d%s   %.2f s" % (
            self.pos + 1, total, "" if d.done else "+", self.pos / self.fps))

    def tick(self):
        d = self.decoder
        if d and d.error:
            self.time_label.configure(text="error: " + d.error[:40])
        elif d and self.playing and d.frames:
            now = time.perf_counter()
            if self._last_tick is None:
                self._last_tick = now
            adv = int((now - self._last_tick) * self.fps * SPEEDS[self.speed_i])
            if adv > 0:
                self._last_tick += adv / (self.fps * SPEEDS[self.speed_i])
                nxt = self.pos + adv
                if nxt >= len(d.frames):
                    if not d.done:
                        nxt = len(d.frames) - 1          # (waiting for the decoder)
                    elif self.loop.get():
                        nxt = 0
                    else:
                        nxt = len(d.frames) - 1
                        self.playing = False
                        self.play_btn.configure(text="▶ Play")
                self.pos = nxt
            self.show()
        elif d and d.frames and self._photo is None:
            self.show()
        self.root.after(8, self.tick)

    # ---- keep / throwaway ----

    def toggle_keep(self):
        self.toggle_status("keep")

    def toggle_throw(self):
        self.toggle_status("throw")

    def toggle_highlight(self):
        self.toggle_status("highlight")

    def toggle_status(self, status):
        """Moves the selected video to `status`'s folder (keep/, keep/highlights/, throwaway/), or back to recordings/ when it is there
        already; then on to the next video."""
        sel = self.listbox.curselection()
        if not sel:
            return
        name, path, current = self.videos[sel[0]]
        new = None if current == status else status
        target = FOLDERS[new] / path.name
        if self.decoder and self.decoder.path == path:
            self.close_video()
        self.release(path)                              # (ffmpeg must let go of the file first)
        try:
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.exists():
                target.unlink()
            shutil.move(str(path), str(target))
        except OSError as e:
            messagebox.showerror("Move", "Could not move %s:\n%s" % (path.name, e))
            return
        idx = sel[0]
        self.refresh_list(keep_name=name)
        still_listed = any(v[0] == name for v in self.videos)
        if new and not still_listed:
            self.select(idx)                            # (it left the list: the next one takes its place)
        elif new:
            self.move_selection(1)                      # (marked: on to the next video)
        else:
            self.open_selected()

    def copy_title(self):
        """The selected video's title (its file name without .mp4, the test script's name) to the clipboard."""
        name = self.current_name()
        if not name:
            return
        self.root.clipboard_clear()
        self.root.clipboard_append(name)
        self.root.update()                  # (the clipboard keeps it after the window closes)
        self.copy_btn.config(text="✓ Copied")
        self.root.after(1200, lambda: self.copy_btn.config(text="⧉ Copy title (C)"))

    def refresh_videos(self):
        """Every loaded video whose file changed on disk (re-recorded, or moved away) is dropped, so it is read again;
        the list is rescanned (new videos appear) and the current video reopened if it was one of them."""
        name = self.current_name()
        current = self.decoder.path if self.decoder else None
        changed = [path for path, d in self.cache.items() if file_stamp(path) != d.stamp]
        if current in changed:
            self.close_video()
        for path in changed:
            self.release(path)
        self.refresh_list(keep_name=name)
        if self.decoder is None:
            self.open_selected()
        self.title_label.configure(text=self.title_label.cget("text").split("   ⟳")[0]
                                   + "   ⟳ %d changed video%s refreshed" % (len(changed), "" if len(changed) == 1 else "s"))

    def open_folder(self):
        folder = KEEP if KEEP.is_dir() else RECORDINGS
        if os.name == "nt":
            os.startfile(str(folder))

    def quit(self):
        self.close_video()
        for d in self.cache.values():
            d.stop()
        self.root.destroy()


def main():
    if not RECORDINGS.is_dir():
        print("No recordings folder: %s" % RECORDINGS)
        return 1
    root = tk.Tk()
    try:
        ttk.Style(root).theme_use("vista" if os.name == "nt" else "clam")
    except tk.TclError:
        pass
    App(root)
    root.mainloop()
    return 0


if __name__ == "__main__":
    sys.exit(main())
