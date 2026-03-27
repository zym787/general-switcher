import os
import subprocess
import shutil
import argparse
import glob
import zipfile
import sys
import datetime
import logging

# ========================
# 1. 配置区域 (只需修改这里)
# ========================

# 项目配置定义所有TARGETS 对应Keil不同的TARGET
TARGETS = [
    "O_901",
    "O_906",
    "O_909",
    "A_901",
    "A_906",
    "A_909",
    "B_901",
    "B_906",
    "C_901",
]
OBJ_DIR = "OBJ"  # 编译产物目录
SOFTWARE_NAME = f"QHF_v1.3.1"  # 软件名称

# 默认路径参数  注意路径!!!必须用\反斜杠
DEFAULT_KEIL_PATH = "D:\\toolchain\\Keil_v5\\UV4\\UV4.exe"
DEFAULT_PROJECT_PATH = ".\\USER\\QHF.uvprojx"
DEFAULT_VERSION_PATH = ".\\Version"
DEFAULT_OUTPUT_PATH = ".\\Version\\output"

# 目标名称到Version文件夹的映射关系
TARGET_VERSION_MAP = {
    "O_901": ["", "901", ""],
    "O_906": ["", "906", ""],
    "O_909": ["", "909", ""],
    "A_901": ["A", "901", "A状态IN高OUT高_B状态IN低OUT低"],
    "A_906": ["A", "906", "A状态BI高BO低_B状态BI低BO高"],
    "A_909": ["A", "909", "A状态BI高BO低_B状态BI低BO高"],
    "B_901": ["B", "901", "A状态IN高OUT低_B状态IN低OUT高"],
    "B_906": ["B", "906", "A状态BI高BO高_B状态BI低BO低"],
    "C_901": ["C", "901", "带灯输出A状态IN高OUT高_B状态IN低OUT低_半通道全低_错误全高"],
}

# 确保控制台输入支持中文（Windows系统需要）
if os.name == "nt":
    import io

    sys.stdin = io.TextIOWrapper(sys.stdin.buffer, encoding="utf-8")
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8")


# ========================
# 2. 统一日志接口 (核心优化)
# ========================
def init_logger(log_file_path):
    """初始化日志系统，输出到文件和控制台"""
    logger = logging.getLogger("release_logger")
    logger.setLevel(logging.INFO)

    # 控制台处理器
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(logging.Formatter("%(message)s"))
    logger.addHandler(console_handler)

    # 文件处理器
    file_handler = logging.FileHandler(log_file_path, encoding="utf-8")
    file_handler.setFormatter(logging.Formatter("%(message)s"))
    logger.addHandler(file_handler)

    return logger


def log_info(message):
    """统一日志接口：记录信息并刷新控制台"""
    logger = logging.getLogger("release_logger")
    logger.info(message)
    sys.stdout.flush()


# ========================
# 3. 工具函数模块 (使用统一日志接口)
# ========================
def clear_obj_folder(obj_dir):
    """清空OBJ文件夹内的所有文件（保留文件夹本身）"""
    if not os.path.exists(obj_dir):
        log_info(f"⚠️警告: OBJ目录不存在 ({obj_dir})，将创建它")
        os.makedirs(obj_dir, exist_ok=True)
        return

    for file in os.listdir(obj_dir):
        file_path = os.path.join(obj_dir, file)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            log_info(f"⚠️警告: 无法删除 {file_path} - {str(e)}")

    log_info(f"✅ 已清空 OBJ 文件夹: {obj_dir}")


def after_run_clear():
    for file in os.listdir("."):
        if file == f"QHF.hex" or file == f"keil_compile_log.txt":
            os.remove(file)
            log_info(f"🗑️ 已删除: {file}")


def get_hex_file(OBJ_DIR):
    """在OBJ目录中查找第一个.hex文件"""
    for file in os.listdir(OBJ_DIR):
        if file.endswith(".hex"):
            return os.path.join(OBJ_DIR, file)
    log_info(f"❌ 错误: OBJ目录中未找到.hex文件: {OBJ_DIR}")
    return None


def validate_change_note(note):
    """验证变更记录是否合法（只允许字母、数字和下划线）"""
    if not all(c.isalnum() or c == "_" for c in note):
        raise ValueError("变更记录只允许使用字母、数字和下划线 (例如: fix_bug)")
    return note


def rename_and_copy_file(
    hex_path, version_dir, base_name, version_number, date_str, target, change_note
):
    """重命名.hex文件并复制到Version目录"""
    # 构建新文件名：QHF_v1.3.1-r版次号_date_target_(变更记录).hex
    version_flag = TARGET_VERSION_MAP.get(target)
    log_info(
        f"👌 {target}  Board:|{version_flag[1]}|  Flag:|{version_flag[0]}|  Note:|{version_flag[2]}|"
    )

    new_name = f"{base_name}{version_flag[0]}-r{version_number}_{version_flag[1]}_{date_str}_({change_note})_{version_flag[2]}.hex"
    new_path = os.path.join(version_dir, new_name)

    # 重命名并复制文件
    shutil.copy2(hex_path, new_path)
    log_info(f"✅ 已复制: {hex_path} -> {new_path}")
    return new_path


def rename_and_copy_keil_complie_log_file(
    version_dir, base_name, version_number, date_str, target
):
    version_flag = TARGET_VERSION_MAP.get(target)
    log_info(
        f"👌 {target}  Board:|{version_flag[1]}|  Flag:|{version_flag[0]}|  Note:|{version_flag[2]}|"
    )

    new_name = f"{base_name}{version_flag[0]}-r{version_number}_{version_flag[1]}_{date_str}_keil_compile_log.txt"
    new_path = os.path.join(version_dir, new_name)

    """在OBJ目录中查找keil_compile_log文件"""
    for file in os.listdir("."):
        if file == f"keil_compile_log.txt":
            shutil.copy2(file, new_path)
            log_info(f"✅ 已复制: {file} -> {new_path}")
            return new_path


def zip_obj_folder(
    obj_dir, output_dir, base_name, version_number, date_str, target, change_note
):
    version_flag = TARGET_VERSION_MAP.get(target)
    log_info(
        f"👌 {target}  Board:|{version_flag[1]}|  Flag:|{version_flag[0]}|  Note:|{version_flag[2]}|"
    )

    """压缩OBJ目录为zip文件并存放到Version目录"""
    # 构建压缩包名：QHF_v1.3.1+版次号_date_target_(变更记录).zip
    zip_name = f"{base_name}{version_flag[0]}-r{version_number}_{version_flag[1]}_{date_str}_({change_note})_{version_flag[2]}_output.zip"
    zip_path = os.path.join(output_dir, zip_name)

    # 创建压缩包
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        for root, _, files in os.walk(obj_dir):
            for file in files:
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, obj_dir)
                zipf.write(file_path, arcname)

    log_info(f"✅ 已压缩: {obj_dir} -> {zip_path}")
    return zip_path


def compile_target(keil_path, project_path, target):
    """调用编译工具编译指定target"""
    autopiler = "Keil-Autopiler.exe"

    # 使用统一日志接口记录
    log_info(f"\n🔃开始编译 target: {target}")
    log_info(f"🛠️使用工具: {autopiler} {keil_path} {project_path} {target}")

    # 调用编译工具
    try:
        result = subprocess.run(
            [autopiler, keil_path, project_path, target],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        log_info(f"✅ 编译成功: {target}")
        return result.stdout
    except subprocess.CalledProcessError as e:
        log_info(f"❌ 编译失败: {target} (错误代码: {e.returncode})")
        return result.stdout


def main():
    # ========== 用户输入配置 (带缺省值) ==========
    log_info("\n==================== 输入提示 ====================")
    log_info("         按回车使用默认值 (括号内为默认值)")
    log_info("====================================================")

    keil_path = (
        input(
            f"👉 请输入Keil安装路径 (例如: C:/Keil5, 缺省为: {DEFAULT_KEIL_PATH}): "
        ).strip()
        or DEFAULT_KEIL_PATH
    )
    project_path = (
        input(
            f"👉 请输入工程文件路径 (例如: USER/project.uvprojx, 缺省为: {DEFAULT_PROJECT_PATH}): "
        ).strip()
        or DEFAULT_PROJECT_PATH
    )
    version_path = (
        input(
            f"👉 请输入Version文件夹路径 (例如: Version, 缺省为: {DEFAULT_VERSION_PATH}): "
        ).strip()
        or DEFAULT_VERSION_PATH
    )
    version_number = input("👉 请输入版次号 (例如: 1): ")
    # 重要：变更记录建议使用英文或数字（避免中文导致的文件名问题）
    change_note = input("👉 请输入简短变更记录 (建议使用英文或数字, 例如: fix_bug): ")

    #     # 验证变更记录
    #     try:
    #         change_note = validate_change_note(change_note)
    #     except ValueError as e:
    #         print(f"❌ 错误: {e}", flush=True)
    #         print("请重新运行脚本并输入合法的变更记录", flush=True)
    #         sys.exit(1)

    # ========== 获取当前日期 (格式: YYMMDD) ==========
    # 例如: 260327 (2026年3月27日)
    current_date = datetime.datetime.now().strftime("%y%m%d")
    log_info(f"📅当前日期: {current_date} (格式: YYMMDD)")

    # ========== 确认目录结构 ==========
    base_name = f"{SOFTWARE_NAME}"

    # 创建Version目录 (如果不存在)
    os.makedirs(version_path, exist_ok=True)
    os.makedirs(DEFAULT_OUTPUT_PATH, exist_ok=True)

    total_targets = len(TARGETS)

    # ========== 初始化日志文件 ==========
    log_file = os.path.join(version_path, "release_version.txt")
    logger = init_logger(log_file)

    # 写入日志头部信息
    log_info(
        f"=== 编译日志开始 ({datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}) ==="
    )
    log_info(f"Keil路径: {keil_path}")
    log_info(f"工程路径: {project_path}")
    log_info(f"版次号: {version_number}")
    log_info(f"变更记录: {change_note}")
    log_info(f"日期: {current_date}")
    log_info(f"Targets: {', '.join(TARGETS)}\n")

    # ========== 处理每个target ==========
    for idx, target in enumerate(TARGETS):
        try:
            # 0. 显示进度条
            progress = f"[{'#' * (idx+1)}{'.' * (total_targets - idx - 1)}] {idx+1}/{total_targets}"
            log_info(f"\n")
            log_info(
                f"\n**********************************************************************************************"
            )
            log_info(f"{progress} - 正在编译 target: {target}")

            # 1. 清空OBJ文件夹
            clear_obj_folder(OBJ_DIR)
            log_info(f"=========== 1 已清空OBJ目录: {OBJ_DIR}===========")

            # 2. 编译target
            compile_target(keil_path, project_path, target)
            log_info(f"=========== 2 已编译target: {target}===========")

            # 3. 复制编译日志文件
            rename_and_copy_keil_complie_log_file(
                version_path, base_name, version_number, current_date, target
            )
            log_info(f"=========== 3 已复制编译日志文件===========")

            # 4. 查找.hex文件
            hex_file = get_hex_file(OBJ_DIR)
            if not hex_file:
                raise FileNotFoundError(f"😵未找到.hex文件 (在{OBJ_DIR}中)")
            log_info(f"=========== 4 已找到.hex文件: {hex_file}===========")

            # 5. 重命名并复制文件
            new_hex_path = rename_and_copy_file(
                hex_file,
                version_path,
                base_name,
                version_number,
                current_date,
                target,
                change_note,
            )
            log_info(f"=========== 5 已重命名并复制文件: {new_hex_path}===========")

            # 6. 压缩OBJ目录
            zip_path = zip_obj_folder(
                OBJ_DIR,
                DEFAULT_OUTPUT_PATH,
                base_name,
                version_number,
                current_date,
                target,
                change_note,
            )
            log_info(f"=========== 6 已压缩OBJ目录并复制文件: {zip_path}===========")

            # 7. 清空OBJ文件夹
            after_run_clear()
            log_info(f"=========== 7 已清除hex及log文件===========")

        except Exception as e:
            log_info(f"❌ 处理target {target} 失败: {str(e)}")
            sys.exit(1)

    log_info(
        f"\n=== 编译日志结束 ({datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}) ==="
    )
    log_info("\n✅ 所有target处理完成！")
    log_info(f"💯 产物已存放在: {version_path}")
    log_info(f"📄 编译日志文件: {log_file}")
    log_info("示例文件名: QHF_v1.3.1-r1_O_901_260327_(fix_bug).hex")


# def validate_paths(keil_path, project_path, version_base_dir):
#     """验证所有路径是否有效"""
#     # 检查Keil路径
#     if not os.path.exists(keil_path):
#         print(f"❌ 错误: Keil路径无效 - {keil_path}")
#         return False

#     # 检查工程文件
#     if not os.path.isfile(project_path):
#         print(f"❌ 错误: 工程文件不存在 - {project_path}")
#         return False

#     # 检查Version基础路径
#     if not os.path.isdir(version_base_dir):
#         print(f"❌ 错误: Version基础路径无效 - {version_base_dir}")
#         return False

#     return True


# def get_autopiler_path(project_path):
#     """获取Keil-Autopiler.exe的路径"""
#     return os.path.join(os.path.dirname(project_path), "Keil-Autopiler.exe")


# def get_final_version_folder(version_base_dir, target):
#     """获取最终的Version文件夹路径 (带前缀和后缀)"""
#     base_folder = TARGET_VERSION_MAP.get(target)
#     if not base_folder:
#         print(f"❌ 错误: 未找到目标映射 - {target}")
#         return None

#     # 拼接前缀、基础文件夹名和后缀
#     final_folder = VERSION_FOLDER_PREFIX + base_folder + VERSION_FOLDER_SUFFIX
#     return os.path.join(version_base_dir, final_folder)


# def compile_target(keil_path, project_path, target):
#     """编译指定的Keil target，记录log并重命名hex文件"""
#     autopiler = get_autopiler_path(project_path)

#     # 检查工具是否存在
#     if not os.path.exists(autopiler):
#         print(f"❌ 错误: 未找到编译工具 - {autopiler}")
#         return False

#     print(f"编译目标: {target} | 使用Keil: {keil_path}")

#     # 创建log文件路径
#     log_file = os.path.join(os.path.dirname(project_path), f"{target}.log")

#     # 调用编译工具并捕获输出
#     try:
#         result = subprocess.run(
#             [autopiler, keil_path, project_path, target],
#             stdout=subprocess.PIPE,
#             stderr=subprocess.PIPE,
#             text=True,
#             check=True,
#         )

#         # 保存编译log
#         with open(log_file, "w") as f:
#             f.write("=== 编译标准输出 ===\n")
#             f.write(result.stdout)
#             f.write("\n=== 编译错误输出 ===\n")
#             f.write(result.stderr)

#         print(f"✅ 编译成功: {target} | Log已保存: {log_file}")

#         # 重命名hex文件
#         if rename_hex_file(project_path, target):
#             print(f"  ✅ Hex文件已重命名为: {target}.hex")
#         else:
#             print(f"  ⚠️ 警告: 未找到Hex文件，无法重命名")

#         return True
#     except subprocess.CalledProcessError as e:
#         # 保存错误log
#         error_log = os.path.join(os.path.dirname(project_path), f"{target}_error.log")
#         with open(error_log, "w") as f:
#             f.write(f"编译失败 (返回码: {e.returncode})\n")
#             f.write("=== 标准输出 ===\n")
#             f.write(e.stdout)
#             f.write("\n=== 标准错误 ===\n")
#             f.write(e.stderr)

#         print(
#             f"❌ 编译失败: {target} (错误代码: {e.returncode}) | 错误Log: {error_log}"
#         )
#         return False


# def rename_hex_file(project_path, target):
#     """重命名编译生成的hex文件"""
#     # 查找工程目录下的所有hex文件
#     hex_files = glob.glob(os.path.join(os.path.dirname(project_path), "*.hex"))

#     if not hex_files:
#         return False

#     # 只处理第一个hex文件（通常一个target只生成一个hex）
#     old_hex = hex_files[0]
#     new_hex = os.path.join(os.path.dirname(project_path), f"{target}.hex")

#     # 如果目标文件已存在，先删除
#     if os.path.exists(new_hex):
#         os.remove(new_hex)

#     # 重命名文件
#     os.rename(old_hex, new_hex)
#     return True


# def move_obj_files(project_path, target, version_base_dir):
#     """将OBJ文件移动到对应的Version文件夹 (带前缀后缀)"""
#     # 获取最终的Version文件夹路径
#     target_version_dir = get_final_version_folder(version_base_dir, target)
#     if not target_version_dir:
#         return False

#     # 确保目标目录存在
#     os.makedirs(target_version_dir, exist_ok=True)
#     print(f"确保目标目录存在: {target_version_dir}")

#     # 源OBJ文件夹
#     obj_folder = os.path.join(os.path.dirname(project_path), "OBJ")

#     # 检查OBJ文件夹是否存在
#     if not os.path.isdir(obj_folder):
#         print(f"❌ 错误: OBJ文件夹不存在 - {obj_folder}")
#         return False

#     # 移动文件
#     moved_count = 0
#     for filename in os.listdir(obj_folder):
#         if os.path.isfile(os.path.join(obj_folder, filename)):
#             source = os.path.join(obj_folder, filename)
#             dest = os.path.join(target_version_dir, filename)

#             # 如果目标文件已存在，先删除
#             if os.path.exists(dest):
#                 os.remove(dest)

#             shutil.move(source, dest)
#             moved_count += 1

#     print(f"✅ 移动文件: {moved_count} 个文件到 {target_version_dir}")

#     # 打包这个文件夹
#     if pack_version_folder(version_base_dir, target):
#         print(f"✅ 已打包: {target}.zip")

#     return True


# def pack_version_folder(version_base_dir, target):
#     """将Version文件夹打包成ZIP文件"""
#     # 获取最终的文件夹路径
#     final_folder = get_final_version_folder(version_base_dir, target)
#     if not final_folder or not os.path.isdir(final_folder):
#         print(f"❌ 错误: 无法打包文件夹 - {final_folder}")
#         return False

#     # 生成ZIP文件名
#     zip_file = os.path.join(version_base_dir, f"{target}.zip")

#     try:
#         # 创建ZIP文件
#         with zipfile.ZipFile(zip_file, "w", zipfile.ZIP_DEFLATED) as zipf:
#             # 遍历文件夹中的所有文件
#             for root, _, files in os.walk(final_folder):
#                 for file in files:
#                     file_path = os.path.join(root, file)
#                     # 保存相对路径，避免包含完整路径
#                     arcname = os.path.relpath(file_path, final_folder)
#                     zipf.write(file_path, arcname)

#         print(f"✅ 已打包: {zip_file}")
#         return True
#     except Exception as e:
#         print(f"❌ 打包失败: {e}")
#         return False


# ========================
# 3. 主程序
# ========================


# def main():
#     """主程序入口"""
#     # 解析命令行参数
#     parser = argparse.ArgumentParser(description="自动化编译Keil工程")
#     parser.add_argument(
#         "--keil-path", required=True, help="Keil安装路径 (例如: C:\\Keil_v5\\UV4)"
#     )
#     parser.add_argument(
#         "--project-path",
#         required=True,
#         help="工程文件路径 (例如: C:\\Project\\MyProject.uvprojx)",
#     )
#     parser.add_argument(
#         "--version-base-dir",
#         required=True,
#         help="Version文件夹基础路径 (例如: C:\\Project\\Version)",
#     )

#     args = parser.parse_args()

#     # 验证路径
#     if not validate_paths(args.keil_path, args.project_path, args.version_base_dir):
#         return

#     # 遍历所有目标
#     for target in TARGETS:
#         print(f"\n{'='*50}")
#         print(f"正在处理目标: {target}")
#         print(f"{'='*50}")

#         # 1. 编译目标
#         if not compile_target(args.keil_path, args.project_path, target):
#             continue

#         # 2. 移动OBJ文件
#         move_obj_files(args.project_path, target, args.version_base_dir)

#     print(f"\n{'='*50}")
#     print("所有目标处理完成!")
#     print(f"{'='*50}")


if __name__ == "__main__":
    main()
