with open("disk.img", "wb") as f:
    f.write(b"\x00" * 4 * 1024 * 1024) # 4MB