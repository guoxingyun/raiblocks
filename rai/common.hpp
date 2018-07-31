#pragma once

#include <rai/lib/blocks.hpp>
#include <rai/node/utility.hpp>

#include <boost/property_tree/ptree.hpp>

#include <unordered_map>

#include <blake2/blake2.h>

#include <string>

namespace boost
{
template <>
struct hash<rai::uint256_union>
{
	size_t operator() (rai::uint256_union const & value_a) const
	{
		std::hash<rai::uint256_union> hash;
		return hash (value_a);
	}
};
}
namespace rai
{
const uint8_t protocol_version = 0x08;
const uint8_t protocol_version_min = 0x07;

class block_store;
/**
 * Determine the balance as of this block
 */
class balance_visitor : public rai::block_visitor
{
public:
	balance_visitor (MDB_txn *, rai::block_store &);
	virtual ~balance_visitor () = default;
	void compute (rai::block_hash const &);
	void send_block (rai::send_block const &) override;
	void receive_block (rai::receive_block const &) override;
	void open_block (rai::open_block const &) override;
	void change_block (rai::change_block const &) override;
	void state_block (rai::state_block const &) override;
	MDB_txn * transaction;
	rai::block_store & store;
	rai::block_hash current;
	rai::uint128_t result;
};

/**
 * Determine the amount delta resultant from this block
 * 确定从这个块中获取交易量
 */
class amount_visitor : public rai::block_visitor
{
public:
	amount_visitor (MDB_txn *, rai::block_store &);
	virtual ~amount_visitor () = default;
	void compute (rai::block_hash const &);
	void send_block (rai::send_block const &) override;
	void receive_block (rai::receive_block const &) override;
	void open_block (rai::open_block const &) override;
	void change_block (rai::change_block const &) override;
	void state_block (rai::state_block const &) override;
	void from_send (rai::block_hash const &);
	MDB_txn * transaction;
	rai::block_store & store;
	rai::block_hash current;
	rai::uint128_t result;
};

/**
 * Determine the representative for this block
 * 获取快中代表
 */
class representative_visitor : public rai::block_visitor
{
public:
	representative_visitor (MDB_txn * transaction_a, rai::block_store & store_a);
	virtual ~representative_visitor () = default;
	void compute (rai::block_hash const & hash_a);
	void send_block (rai::send_block const & block_a) override;
	void receive_block (rai::receive_block const & block_a) override;
	void open_block (rai::open_block const & block_a) override;
	void change_block (rai::change_block const & block_a) override;
	void state_block (rai::state_block const & block_a) override;
	MDB_txn * transaction;
	rai::block_store & store;
	rai::block_hash current;
	rai::block_hash result;
};

/**
 * A key pair. The private key is generated from the random pool, or passed in
 * as a hex string. The public key is derived using ed25519.
 * *一个密钥对。私钥由随机池生成，或传入
 * *作为十六进制字符串。公钥使用ed25519派生。
 */
class keypair
{
public:
	keypair ();
	keypair (std::string const &);
	rai::public_key pub;
	rai::raw_key prv;
};

std::unique_ptr<rai::block> deserialize_block (MDB_val const &);

/**
 * Latest information about an account
 * 账户最新的信息
 */
class account_info
{
public:
	account_info ();
	account_info (MDB_val const &);
	account_info (rai::account_info const &) = default;
	account_info (rai::block_hash const &, rai::block_hash const &, rai::block_hash const &, rai::amount const &, uint64_t, uint64_t);
	account_info (rai::block_hash const &, rai::block_hash const &, rai::block_hash const &, rai::amount const &, uint64_t, uint64_t,uint64_t,rai::token_amount const &);
	void serialize (rai::stream &) const;
	void token_serialize (rai::stream &) const;
	bool deserialize (rai::stream &);
	bool token_deserialize (rai::stream &);
	bool operator== (rai::account_info const &) const;
	bool operator!= (rai::account_info const &) const;
	rai::mdb_val val () const;
	rai::block_hash head;
	rai::block_hash rep_block;
	rai::block_hash open_block;
	rai::amount balance;
	/** Seconds since posix epoch */
	uint64_t modified;
	uint64_t block_count;
	uint64_t  token_name;
	rai::token_amount token_balance;
};

/**
 * Information on an uncollected send, source account, amount, target account.
 */
class pending_info
{
public:
	pending_info ();
	pending_info (MDB_val const &);
	pending_info (rai::account const &, rai::amount const &);
	pending_info (rai::account const &, uint64_t, rai::token_amount const &);
	void serialize (rai::stream &) const;
	void token_serialize (rai::stream &) const;
	bool deserialize (rai::stream &);
	bool token_deserialize (rai::stream &);
	bool operator== (rai::pending_info const &) const;
	rai::mdb_val val () const;
	rai::account source;
	rai::amount amount;
	uint64_t  token_name;
	rai::token_amount token_amount;
};
class pending_key
{
public:
	pending_key (rai::account const &, rai::block_hash const &);
	pending_key (MDB_val const &);
	void serialize (rai::stream &) const;
	bool deserialize (rai::stream &);
	bool operator== (rai::pending_key const &) const;
	rai::mdb_val val () const;
	rai::account account;
	rai::block_hash hash;
};
class block_info
{
public:
	block_info ();
	block_info (MDB_val const &);
	block_info (rai::account const &, rai::amount const &);
	void serialize (rai::stream &) const;
	bool deserialize (rai::stream &);
	bool operator== (rai::block_info const &) const;
	rai::mdb_val val () const;
	rai::account account;
	rai::amount balance;
};
class block_counts
{
public:
	block_counts ();
	size_t sum ();
	size_t send;
	size_t receive;
	size_t open;
	size_t change;
	size_t state;
};
class vote
{
public:
	vote () = default;
	vote (rai::vote const &);
	vote (bool &, rai::stream &);
	vote (bool &, rai::stream &, rai::block_type);
	vote (rai::account const &, rai::raw_key const &, uint64_t, std::shared_ptr<rai::block>);
	vote (MDB_val const &);
	rai::uint256_union hash () const;
	bool operator== (rai::vote const &) const;
	bool operator!= (rai::vote const &) const;
	void serialize (rai::stream &, rai::block_type);
	void serialize (rai::stream &);
	std::string to_json () const;
	// Vote round sequence number
	// 投票的序列号
	uint64_t sequence;
	std::shared_ptr<rai::block> block;
	// Account that's voting
	rai::account account;
	// Signature of sequence + block hash
	// 还对（序列号和块hash）进行签名
	rai::signature signature;
};
enum class vote_code
{
	invalid, // Vote is not signed correctly,无效签名
	replay, // Vote does not have the highest sequence number, it's a replay，投票没有最高的序列号，这是重播
	vote // Vote has the highest sequence number，最高序列的投票
};
class vote_result
{
public:
	rai::vote_code code;
	std::shared_ptr<rai::vote> vote;
};

enum class process_result
{
	progress, // Hasn't been seen before, signed correctly,OK块
	bad_signature, // Signature was bad, forged or transmission error，无效的签名
	old, // Already seen and was valid，之前存在的有效块
	negative_spend, // Malicious attempt to spend a negative amount,恶意企图花费一个负数金额
	fork, // Malicious fork based on previous,双花
	unreceivable, // Source block doesn't exist or has already been received，源块不存在，或者已经被接收过了
	gap_previous, // Block marked as previous is unknown,,间隙块，找不到前区块
	gap_source, // Block marked as source is unknown,再数据库没有找到发送方的源区块
	state_block_disabled, // Awaiting state block canary block，没有打开能state块的权限
	not_receive_from_send, // Receive does not have a send source，打包接收块没有发送方？
	account_mismatch, // Account number in open block doesn't match send destination，open块中的账户和发送方的目标地址不一致
	opened_burn_account, // The impossible happened, someone found the private key associated with the public key '0'.销毁账号被破解出私钥的特殊处理
	balance_mismatch, // Balance and amount delta don't match，余额和发送量不一致
	block_position // This block cannot follow the previous block，该块不能链接到前区块下,除了open块其他都允许返回true
};
class process_return
{
public:
	rai::process_result code;
	rai::account account;
	rai::amount amount;
	rai::account pending_account;
	boost::optional<bool> state_is_send;
};
enum class tally_result
{
	vote,
	changed,
	confirm
};
class votes
{
public:
	votes (std::shared_ptr<rai::block>);
	rai::tally_result vote (std::shared_ptr<rai::vote>);
	bool uncontested ();
	// Root block of fork,分叉双花前的块hash
	rai::block_hash id;
	// All votes received by account，账户接收的所有选票
	std::unordered_map<rai::account, std::shared_ptr<rai::block>> rep_votes;
};
extern rai::keypair const & zero_key;
extern rai::keypair const & test_genesis_key;
extern rai::account const & rai_test_account;
extern rai::account const & rai_beta_account;
extern rai::account const & rai_live_account;
extern std::string const & rai_test_genesis;
extern std::string const & rai_beta_genesis;
extern std::string const & rai_live_genesis;
extern std::string const & genesis_block;
extern rai::account const & genesis_account;
extern rai::account const & burn_account;
extern rai::uint128_t const & genesis_amount;
// A block hash that compares inequal to any real block
// 在表中没找到该块hash
extern rai::block_hash const & not_a_block;
// An account number that compares inequal to any real account number
// 表中没该账户
extern rai::block_hash const & not_an_account;
class genesis
{
public:
	explicit genesis ();
	void initialize (MDB_txn *, rai::block_store &) const;
	rai::block_hash hash () const;
	std::unique_ptr<rai::open_block> open;
};
}
