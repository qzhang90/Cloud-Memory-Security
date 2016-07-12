#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>

#include "elf.h"

/* 
 * elf_open - Map a binary into the address space and extract the
 * locations of the static and dynamic symbol tables and their string
 * tables. Return this information in a Elf object file handle that will
 * be passed to all of the other elf functions.
 */
Elf_obj *elf_open(char *filename) 
{
    int i, fd;
    struct stat sbuf;
    Elf_obj *ep;
    Elf64_Shdr *shdr;

    if ((ep = (Elf_obj *) malloc(sizeof(Elf_obj))) == NULL) {
	fprintf(stderr, "Malloc failed: %s\n", strerror(errno));
	exit(1);
    }

    /* Do some consistency checks on the binary */
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
	fprintf(stderr,	"Can't open \"%s\": %s\n", filename, strerror(errno));
	exit(1);
    }
    if (fstat(fd, &sbuf) == -1) {
	fprintf(stderr, "Can't stat \"%s\": %s\n", filename, strerror(errno));
	exit(1);
    }
    if (sbuf.st_size < sizeof(Elf64_Ehdr)) {
	fprintf(stderr, "\"%s\" is not an ELF binary object\n",	filename);
	exit(1);
    }

    /* It looks OK, so map the Elf binary into our address space */
    ep->mlen = sbuf.st_size;
    ep->maddr = mmap(NULL, ep->mlen, PROT_READ, MAP_SHARED, fd, 0);
    if (ep->maddr == (void *)-1) {
	fprintf(stderr,	"Can't mmap \"%s\": %s\n", filename, strerror(errno));
	exit(1);
    }
    close(fd);

    /* The Elf binary begins with the Elf header */
    ep->ehdr = ep->maddr;

    /* Make sure that this is an Elf binary */ 
    if (strncmp(ep->ehdr->e_ident, ELFMAG, SELFMAG)) {
	fprintf(stderr, "\"%s\" is not an ELF binary object\n",	filename);
	exit(1);
    }

    /* 
     * Find the static and dynamic symbol tables and their string
     * tables in the the mapped binary. The sh_link field in symbol
     * table section headers gives the section index of the string
     * table for that symbol table.
     */
    shdr = (Elf64_Shdr *)(ep->maddr + ep->ehdr->e_shoff);
    for (i = 0; i < ep->ehdr->e_shnum; i++) {
	if (shdr[i].sh_type == SHT_SYMTAB) {   /* Static symbol table */
	    ep->symtab = (Elf64_Sym *)(ep->maddr + shdr[i].sh_offset);
	    ep->symtab_end = (Elf64_Sym *)((char *)ep->symtab + shdr[i].sh_size);
	    ep->strtab = (char *)(ep->maddr + shdr[shdr[i].sh_link].sh_offset);
	}
	if (shdr[i].sh_type == SHT_DYNSYM) {   /* Dynamic symbol table */
	    ep->dsymtab = (Elf64_Sym *)(ep->maddr + shdr[i].sh_offset);
	    ep->dsymtab_end = (Elf64_Sym *)((char *)ep->dsymtab + shdr[i].sh_size);
	    ep->dstrtab = (char *)(ep->maddr + shdr[shdr[i].sh_link].sh_offset);
	}
    }
    return ep;
}

/* 
 * elf_close - Free up the resources of an  elf object
 */
void elf_close(Elf_obj *ep) 
{
    if (munmap(ep->maddr, ep->mlen) < 0) {
	perror("munmap");
	exit(1);
    }
    free(ep);
}

/*
 * elf_symname - Return ASCII name of a static symbol
 */
char *elf_symname(Elf_obj *ep, Elf64_Sym *sym)
{
    return &ep->strtab[sym->st_name];
}

/*
*elf_symvalue - Return value of a static symbol
*/
unsigned long elf_symvalue(Elf_obj *ep, Elf64_Sym *sym)
{
    //return ep->strtab[sym->st_value];
    return sym->st_value;
}

/*
*elf_symsize - Return size of a static symbol
*/
unsigned long elf_symsize(Elf_obj *ep, Elf64_Sym *sym)
{
    //return ep->strtab[sym->st_size];
    return sym->st_size;
}

/*
 * elf_dsymname - Return ASCII name of a dynamic symbol
 */ 
char *elf_dsymname(Elf_obj *ep, Elf64_Sym *sym)
{
    return &ep->dstrtab[sym->st_name];
}

/*
 * elf_firstsym - Return ptr to first symbol in static symbol table
 */
Elf64_Sym *elf_firstsym(Elf_obj *ep)
{
    return ep->symtab;
}

/*
 * elf_nextsym - Return ptr to next symbol in static symbol table,
 * or NULL if no more symbols.
 */
Elf64_Sym *elf_nextsym(Elf_obj *ep, Elf64_Sym *sym)
{
    sym++;
    if (sym < ep->symtab_end)
	return sym;
    else
	return NULL;
}

/*
 * elf_firstdsym - Return ptr to first symbol in dynamic symbol table
 */
Elf64_Sym *elf_firstdsym(Elf_obj *ep)
{
    return ep->dsymtab;
}

/*
 * elf_nextdsym - Return ptr to next symbol in dynamic symbol table,
 * of NULL if no more symbols.
 */ 
Elf64_Sym *elf_nextdsym(Elf_obj *ep, Elf64_Sym *sym)
{
    sym++;
    if (sym < ep->dsymtab_end)
	return sym;
    else
	return NULL;
}

/*
 * elf_isfunc - Return true if symbol is a static function
 */
int elf_isfunc(Elf_obj *ep, Elf64_Sym *sym) 
{
    return ((ELF32_ST_TYPE(sym->st_info) == STT_FUNC) &&
	    (sym->st_shndx != SHT_NULL));
}

/*
 * elf_isdfunc - Return true if symbol is a dynamic function 
 */
int elf_isdfunc(Elf_obj *ep, Elf64_Sym *sym) 
{
    return ((ELF32_ST_TYPE(sym->st_info) == STT_FUNC));
}

void main(void){
	Elf64_Sym *sym;
	Elf_obj *elf_obj = elf_open("fopen");

	printf("elf_obj = %p\n", elf_obj);
	
	sym = elf_firstsym(elf_obj);
	
	do{
		printf("Symbol name = %-30s, value = %-30lx, size = %-30d\n", 
			elf_symname(elf_obj, sym),
			elf_symvalue(elf_obj, sym),
			elf_symsize(elf_obj, sym));
		sym = elf_nextsym(elf_obj, sym);
	}while(sym);

	elf_close(elf_obj);
}
