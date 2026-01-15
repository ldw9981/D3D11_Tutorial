import bpy
import sys
import os
import shutil

IMG_EXTS = {".png", ".jpg", ".jpeg", ".tga", ".tif", ".tiff", ".bmp", ".exr", ".hdr"}

def detect_extension_from_data(data: bytes) -> str:
    """packed_file.data의 magic byte로 실제 이미지 형식 감지"""
    if data.startswith(b'\x89PNG\r\n\x1a\n'):
        return ".png"
    if data.startswith(b'\xff\xd8\xff'):
        return ".jpg" if data[6:11] == b'JFIF' else ".jpeg"
    if data.startswith(b'II*\x00') or data.startswith(b'MM\x00*'):
        return ".tif"
    if len(data) > 18 and data[0:2] == b'\x00\x00' and data[18:] == b'TRUEVISION-XFILE':
        return ".tga"
    if data.startswith(b'\x00\x00\x00\x0cjxl '):
        return ".jxl"
    return ".png"  # fallback

def get_original_basename(img) -> tuple[str, str]:
    """
    img.filepath_raw에서 가능한 원본 파일명과 확장자를 추출
    없으면 img.name을 정리해서 사용
    반환: (safe_name_without_ext, ext)
    """
    if img.filepath_raw:
        abs_path = bpy.path.abspath(img.filepath_raw)
        base = os.path.basename(abs_path)
        if '.' in base:
            name, ext = os.path.splitext(base)
            return name, ext.lower()
    
    # fallback: name에서 숫자 접미사 제거 (예: 이미지 텍스처.004 → 이미지 텍스처)
    name_part = img.name
    while name_part and name_part[-1].isdigit():
        name_part = name_part[:-1]
    if name_part.endswith('.'):
        name_part = name_part[:-1]
    return name_part or "untitled", ".png"

def make_unique_path(directory: str, name: str, ext: str) -> str:
    """중복 방지된 경로 생성"""
    path = os.path.join(directory, name + ext)
    if not os.path.exists(path):
        return path
    
    counter = 1
    while True:
        new_path = os.path.join(directory, f"{name}.{counter:03d}{ext}")
        if not os.path.exists(new_path):
            return new_path
        counter += 1

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []
    if len(argv) < 3:
        print("Usage: blender -b -P fbx_externalize_textures.py -- input.fbx texture_dir output.fbx")
        return

    fbx_in   = os.path.abspath(argv[0])
    tex_dir  = os.path.abspath(argv[1])
    fbx_out  = os.path.abspath(argv[2])

    os.makedirs(tex_dir, exist_ok=True)

    # 새 빈 장면으로 시작
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=fbx_in)

    print(f"Imported FBX: {fbx_in}")
    print(f"Extracting textures to: {tex_dir}")

    for img in bpy.data.images:
        if not img:
            continue

        saved_path = None
        linked_path = None

        # 1. packed (임베디드) 데이터가 있는 경우 → 직접 저장
        pf = getattr(img, "packed_file", None)
        if pf and hasattr(pf, "data") and pf.data:
            data = pf.data

            # 원본 이름 추출
            base_name, fallback_ext = get_original_basename(img)

            # magic byte로 확장자 결정 (더 정확)
            ext = detect_extension_from_data(data)

            dst = make_unique_path(tex_dir, base_name, ext)

            with open(dst, "wb") as f:
                f.write(data)

            saved_path = dst
            print(f"Saved packed texture: {os.path.basename(dst)} (from {img.name})")

        # 2. packed가 아니고 외부 파일을 참조 중인 경우 → 필요 시 복사
        else:
            if img.filepath_raw:
                src_abs = bpy.path.abspath(img.filepath_raw)
                if os.path.isfile(src_abs):
                    base_name = os.path.basename(src_abs)
                    dst = os.path.join(tex_dir, base_name)
                    dst = make_unique_path(tex_dir, *os.path.splitext(base_name))

                    # 선택: 실제로 복사하려면 아래 주석 해제
                    # shutil.copy(src_abs, dst)
                    # print(f"Copied external texture: {base_name} → {os.path.basename(dst)}")

                    # 복사하지 않고 경로만 tex_dir 쪽으로 업데이트하려면:
                    linked_path = dst
                    print(f"External texture found: {base_name} (kept original location)")

        # 3. 저장하거나 연결된 경로를 이미지에 반영
        final_path = saved_path or linked_path or None
        if final_path:
            img.filepath_raw = bpy.path.relpath(final_path)  # 상대 경로로 저장 (FBX export 시 유리)
            try:
                img.reload()
            except:
                pass
        elif img.filepath_raw:
            # 외부 파일이 이미 유효하면 그대로 둠
            pass
        else:
            print(f"Skip (no data, no valid path): {img.name}")

    # 4. 텍스처 임베드 없이 FBX 재내보내기
    bpy.ops.export_scene.fbx(
        filepath=fbx_out,
        embed_textures=False,
        path_mode='RELATIVE',  # 텍스처 경로를 상대 경로로 저장
        apply_unit_scale=False,
        apply_scale_options='FBX_SCALE_NONE',
        bake_space_transform=True
    )

    print("Done.")
    print(f"Textures externalized to: {tex_dir}")
    print(f"New FBX saved: {fbx_out}")

if __name__ == "__main__":
    main()