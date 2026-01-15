import bpy
import os
import sys

# --- 유틸 함수 -----------------------------------------------------------
def image_path_info(img):
    fp = img.filepath if hasattr(img, "filepath") else ""
    return {"filepath": fp}

# --- 메인 리포트 출력 -------------------------------------------------------
def print_material_report():
    # 중간 출력 없음 (헤더도 최소화)
    print("REQUIRED TEXTURE PATHS WITH NODENAME")
    print("=" * 80)

    node_path_list = []  # (NodeName, Path) 튜플 저장

    for m in bpy.data.materials:
        if not m.use_nodes or not m.node_tree:
            continue
        nt = m.node_tree
        tex_nodes = [n for n in nt.nodes if n.type == "TEX_IMAGE"]
        for n in tex_nodes:
            img = n.image
            if img is None:
                continue
            info = image_path_info(img)
            path = info['filepath'].strip()
            if path:  # Path가 있는 경우만 수집
                node_path_list.append((n.name, path))

    # 중복 제거 (같은 NodeName + Path 조합 제거)
    seen = set()
    unique_entries = []
    for node_name, path in node_path_list:
        key = (node_name, path)
        if key not in seen:
            seen.add(key)
            unique_entries.append((node_name, path))

    # 출력
    if not unique_entries:
        print("No texture paths found.")
    else:
        print(f"Total: {len(unique_entries)} unique entries")
        print("-" * 80)
        # NodeName 기준으로 정렬 (원하시면 path로 바꿔도 됨)
        for node_name, path in sorted(unique_entries, key=lambda x: x[0]):
            print(f"NodeName: {node_name}")
            print(f"Path    : {path}")
            print()  # 빈 줄로 구분

    print("=" * 80)
    print("Done.")

# --- 메인 실행 -------------------------------------------------------------
def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []
    if len(argv) >= 1 and argv[0].lower().endswith(".fbx"):
        fbx_in = os.path.abspath(argv[0])
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.import_scene.fbx(
            filepath=fbx_in,
            use_image_search=True
        )
    print_material_report()

if __name__ == "__main__":
    main()