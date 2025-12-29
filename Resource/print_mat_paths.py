import bpy
import os
import sys

# --- 유틸: 노드/링크 탐색 -------------------------------------------------

def socket_label(sock):
    try:
        return sock.name
    except:
        return "<socket>"

def node_label(node):
    return getattr(node, "label", "") or node.name

def image_path_info(img):
    # filepath_raw가 보통 원본(저장용), filepath는 블렌더 내부 경로일 때가 있음
    raw = img.filepath_raw if hasattr(img, "filepath_raw") else ""
    fp  = img.filepath if hasattr(img, "filepath") else ""
    ab_raw = bpy.path.abspath(raw) if raw else ""
    ab_fp  = bpy.path.abspath(fp) if fp else ""

    packed = bool(getattr(img, "packed_file", None))
    # 존재여부는 raw/ fp 중 하나라도 실제 파일이면 True
    exists = False
    for cand in [ab_raw, ab_fp]:
        if cand and os.path.isfile(cand):
            exists = True
            break

    return {
        "filepath_raw": raw,
        "filepath": fp,
        "abspath_raw": ab_raw,
        "abspath": ab_fp,
        "packed": packed,
        "exists": exists,
        "colorspace": getattr(getattr(img, "colorspace_settings", None), "name", ""),
        "alpha_mode": getattr(img, "alpha_mode", ""),
        "size": tuple(getattr(img, "size", (0, 0))),
    }

def guess_usage(material, tex_node):
    """
    텍스처 노드가 어떤 용도로 쓰이는지(대략) 추정:
    - Normal Map 노드 입력으로 가면 NORMAL
    - Principled BSDF Base Color로 가면 BASE_COLOR
    - Roughness/Metallic/AO 등도 링크로 추정
    """
    usage = set()

    def follow_outputs(node):
        # node.outputs[*].links 를 따라 downstream으로 추정
        for out in getattr(node, "outputs", []):
            for link in getattr(out, "links", []):
                to_node = link.to_node
                to_sock = link.to_socket
                if not to_node or not to_sock:
                    continue

                # Normal Map 노드로 들어가면
                if to_node.type == "NORMAL_MAP":
                    usage.add("NORMAL (via Normal Map node)")
                # Principled BSDF로 들어가면 소켓명으로 추정
                if to_node.type == "BSDF_PRINCIPLED":
                    n = socket_label(to_sock).lower()
                    if "base color" in n:
                        usage.add("BASE_COLOR")
                    elif "roughness" in n:
                        usage.add("ROUGHNESS")
                    elif "metallic" in n:
                        usage.add("METALLIC")
                    elif "specular" in n:
                        usage.add("SPECULAR")
                    elif "alpha" in n:
                        usage.add("ALPHA")
                    elif "emission" in n:
                        usage.add("EMISSION")
                    elif "normal" in n:
                        usage.add("NORMAL (direct to Principled)")
                    else:
                        usage.add(f"PRINCIPLED:{socket_label(to_sock)}")

                # 다른 노드들도 한번 더 따라가 보기(간단 1-depth)
                if to_node != node:
                    # AO는 보통 Multiply/MixRGB 거쳐 BaseColor로 들어가기도 해서 너무 깊게는 안 감
                    # 여기서는 1단계만 추가 탐색
                    for out2 in getattr(to_node, "outputs", []):
                        for link2 in getattr(out2, "links", []):
                            tn2 = link2.to_node
                            ts2 = link2.to_socket
                            if tn2 and tn2.type == "BSDF_PRINCIPLED":
                                n2 = socket_label(ts2).lower()
                                if "base color" in n2:
                                    usage.add("BASE_COLOR (via intermediate node)")
                                if "ambient occlusion" in n2:
                                    usage.add("AO")

    follow_outputs(tex_node)
    return sorted(usage) if usage else ["(unknown/unused link)"]

# --- 메인 출력 ------------------------------------------------------------

def print_material_report():
    mats = [m for m in bpy.data.materials if m is not None]
    print("=" * 80)
    print(f"Materials found: {len(mats)}")
    print("=" * 80)

    for m in mats:
        print("\n" + "-" * 80)
        print(f"Material: {m.name}")
        print(f"Use Nodes: {m.use_nodes}")
        if not m.use_nodes or not m.node_tree:
            continue

        nt = m.node_tree
        tex_nodes = [n for n in nt.nodes if n.type == "TEX_IMAGE"]

        print(f"Image Texture nodes: {len(tex_nodes)}")

        for n in tex_nodes:
            img = n.image
            print("\n  [ImageTexture Node]")
            print(f"    NodeName : {n.name}")
            print(f"    Label    : {node_label(n)}")
            if img is None:
                print("    Image    : (None)")
                continue

            info = image_path_info(img)
            print(f"    ImageName: {img.name}")
            print(f"    Packed   : {info['packed']}")
            print(f"    Exists   : {info['exists']}")
            print(f"    PathRaw  : {info['filepath_raw']}")
            print(f"    Path     : {info['filepath']}")
            print(f"    AbsRaw   : {info['abspath_raw']}")
            print(f"    AbsPath  : {info['abspath']}")
            print(f"    ColorSpc : {info['colorspace']}")
            print(f"    AlphaMod : {info['alpha_mode']}")
            print(f"    Size     : {info['size'][0]} x {info['size'][1]}")

            usage = guess_usage(m, n)
            print(f"    Usage    : {', '.join(usage)}")

    print("\n" + "=" * 80)
    print("Done.")
    print("=" * 80)

def main():
    argv = sys.argv
    argv = argv[argv.index("--") + 1:] if "--" in argv else []

    # 인자로 FBX가 들어오면 임포트까지 수행
    if len(argv) >= 1 and argv[0].lower().endswith(".fbx"):
        fbx_in = os.path.abspath(argv[0])
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.import_scene.fbx(filepath=fbx_in)

    print_material_report()

if __name__ == "__main__":
    main()
