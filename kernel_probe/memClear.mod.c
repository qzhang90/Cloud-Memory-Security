#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x809613c0, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x65c3044c, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x4c4fef19, __VMLINUX_SYMBOL_STR(kernel_stack) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{        0, __VMLINUX_SYMBOL_STR(filp_close) },
	{ 0xa843805a, __VMLINUX_SYMBOL_STR(get_unused_fd_flags) },
	{ 0x516f38ba, __VMLINUX_SYMBOL_STR(vfs_fsync) },
	{ 0x1b9aca3f, __VMLINUX_SYMBOL_STR(jprobe_return) },
	{ 0xb1bedeb7, __VMLINUX_SYMBOL_STR(register_jprobe) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0xe3d41a89, __VMLINUX_SYMBOL_STR(vfs_read) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x146f7ae2, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x9166fada, __VMLINUX_SYMBOL_STR(strncpy) },
	{ 0xe16b0d30, __VMLINUX_SYMBOL_STR(unregister_jprobe) },
	{ 0x2acf0feb, __VMLINUX_SYMBOL_STR(vfs_fstat) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x41f67cdd, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x7077a44d, __VMLINUX_SYMBOL_STR(fd_install) },
	{ 0x7e3b92b7, __VMLINUX_SYMBOL_STR(vfs_write) },
	{ 0x1a8e2c06, __VMLINUX_SYMBOL_STR(filp_open) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "B65D67C760F4BE4880C9A56");
