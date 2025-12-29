import bpy
import sys
import os

IMG_EXTS = {".png",".jpg",".jpeg",".tga",".tif",".tiff",".bmp",".exr",".hdr"}

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []
    if len(argv) < 3:
        print("Usage: blender -b -P script.py -- input.fbx texture_dir output.fbx")
        return

    fbx_in   = os.path.abspath(argv[0])
    tex_dir  = os.path.abspath(argv[1])
    fbx_out  = os.path.abspath(argv[2])

    os.makedirs(tex_dir, exist_ok=True)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=fbx_in)

    # ❌ 여기서 unpack_all()을 호출하지 않습니다.
    # bpy.ops.file.unpack_all(method='WRITE_LOCAL')

    # 1) packed(임베디드) 데이터가 있으면 tex_dir로 바로 파일 저장
    for img in bpy.data.images:
        if not img:
            continue

        # 파일 확장자 결정(없으면 png)
        ext = os.path.splitext(img.filepath_raw or "")[1].lower()
        if ext not in IMG_EXTS:
            ext = ".png"

        safe_name = os.path.splitext(img.name)[0]
        dst = os.path.join(tex_dir, safe_name + ext)

        saved = False

        # (A) FBX에 임베디드/packed되어 있으면 raw bytes로 바로 저장
        pf = getattr(img, "packed_file", None)
        if pf and hasattr(pf, "data") and pf.data:
            with open(dst, "wb") as f:
                f.write(pf.data)
            saved = True

        # (B) packed가 아니라면, 원본 파일이 실제로 존재하는 경우 복사/재연결만
        if not saved:
            src = bpy.path.abspath(img.filepath_raw) if img.filepath_raw else ""
            if src and os.path.isfile(src):
                # 외부참조 파일이면 그냥 그 경로를 쓰거나, 필요시 dst로 복사해도 됨
                # 여기선 dst로 복사하지 않고 "dst로 저장"만 목표라 skip 처리
                pass

        if saved:
            img.filepath_raw = dst
            try:
                img.reload()
            except:
                pass
            print("Saved & linked:", dst)
        else:
            print("Skip (no packed data):", img.name)

    # 2) FBX Export (텍스처 임베드 끔)
    bpy.ops.export_scene.fbx(
        filepath=fbx_out,
        embed_textures=False,
        path_mode='RELATIVE'
    )

    print("Done.")
    print("Textures externalized to:", tex_dir)
    print("FBX saved:", fbx_out)

if __name__ == "__main__":
    main()
